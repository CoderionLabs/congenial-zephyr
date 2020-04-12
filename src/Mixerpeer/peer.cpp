/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Example using libnice to negotiate a UDP connection between two clients,
 * possibly on the same network or behind different NATs and/or stateful
 * firewalls.
 *
 * Run two clients, one controlling and one controlled:
 *   simple-example 0 $(host -4 -t A stun.stunprotocol.org | awk '{ print $4 }')
 *   simple-example 1 $(host -4 -t A stun.stunprotocol.org | awk '{ print $4 }')
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include "common.hpp"
//extern "C"{
    #include <nice/agent.h>
    #include <gio/gnetworking.h>
//}

static GMainLoop *gloop;
static GIOChannel* io_stdin;
static guint stream_id;

static const gchar *candidate_type_name[] = {"host", "srflx", "prflx", "relay"};

static const gchar *state_name[] = {"disconnected", "gathering", "connecting",
                                    "connected", "ready", "failed"};

std::map<std::string, std::pair<std::string,std::string>> print_local_data(NiceAgent *agent, guint stream_id,
    guint component_id);
static int parse_remote_data(NiceAgent *agent, guint stream_id,
    guint component_id, char *line);
static void cb_candidate_gathering_done(NiceAgent *agent, guint stream_id,
    gpointer data);
static void cb_new_selected_pair(NiceAgent *agent, guint stream_id,
    guint component_id, gchar *lfoundation,
    gchar *rfoundation, gpointer data);
static void cb_component_state_changed(NiceAgent *agent, guint stream_id,
    guint component_id, guint state,
    gpointer data);
static void cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,
    guint len, gchar *buf, gpointer data);
static gboolean stdin_remote_info_cb (GIOChannel *source, GIOCondition cond,
    gpointer data);
static gboolean stdin_send_data_cb (GIOChannel *source, GIOCondition cond,
    gpointer data);


auto main(int argc, char *argv[]) -> int
{
  NiceAgent *agent;
  gchar *stun_addr = NULL;
  guint stun_port = 0;
  gboolean controlling;

  // Parse arguments
  if (argc > 4 || argc < 2 || argv[1][1] != '\0') {
    fprintf(stderr, "Usage: %s 0|1 stun_addr [stun_port]\n", argv[0]);
    return EXIT_FAILURE;
  }
  controlling = argv[1][0] - '0';
  if (controlling != 0 && controlling != 1) {
    fprintf(stderr, "Usage: %s 0|1 stun_addr [stun_port]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (argc > 2) {
    stun_addr = argv[2];
    if (argc > 3)
      stun_port = atoi(argv[3]);
    else
      stun_port = 3478;

    g_debug("Using stun server '[%s]:%u'\n", stun_addr, stun_port);
  }

  g_networking_init();

  gloop = g_main_loop_new(NULL, FALSE);
#ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd(_fileno(stdin));
#else
  io_stdin = g_io_channel_unix_new(fileno(stdin));
#endif

  // Create the nice agent
  agent = nice_agent_new(g_main_loop_get_context (gloop),
      NICE_COMPATIBILITY_RFC5245);
  if (agent == NULL)
    g_error("Failed to create agent");

  // Set the STUN settings and controlling mode
  if (stun_addr) {
    g_object_set(agent, "stun-server", stun_addr, NULL);
    g_object_set(agent, "stun-server-port", stun_port, NULL);
  }
  g_object_set(agent, "controlling-mode", controlling, NULL);

  // Connect to the signals
  g_signal_connect(agent, "candidate-gathering-done",
      G_CALLBACK(cb_candidate_gathering_done), NULL);
  g_signal_connect(agent, "new-selected-pair",
      G_CALLBACK(cb_new_selected_pair), NULL);
  g_signal_connect(agent, "component-state-changed",
      G_CALLBACK(cb_component_state_changed), NULL);

  // Create a new stream with one component
  stream_id = nice_agent_add_stream(agent, 1);
  if (stream_id == 0) g_error("Failed to add stream");

  // Attach to the component to receive the data
  // Without this call, candidates cannot be gathered
  nice_agent_attach_recv(agent, stream_id, 1,
      g_main_loop_get_context (gloop), cb_nice_recv, NULL);

  // Start gathering local candidates
  if (!nice_agent_gather_candidates(agent,
   stream_id)) g_error("Failed to start candidate gathering");

  g_debug("waiting for candidate-gathering-done signal...");

  // Run the mainloop. Everything else will happen asynchronously
  // when the candidates are done gathering.
  g_main_loop_run (gloop);

  g_main_loop_unref(gloop);
  g_object_unref(agent);
  g_io_channel_unref (io_stdin);

  return EXIT_SUCCESS;
}

static void
cb_candidate_gathering_done(NiceAgent *agent, guint _stream_id,
    gpointer data)
{

  g_debug("SIGNAL candidate gathering done\n");

  // Candidate gathering is done. Send our local candidates on stdout
  std::cout << "Copy this line to remote client:\n";
  std::cout << std::endl;
  auto mymap = print_local_data(agent, _stream_id, 1);
  std::cout << std::endl;
 
  // Listen on stdin for the remote candidate list
  std::cout << "Enter remote data (single line, no wrapping):\n";
  g_io_add_watch(io_stdin, G_IO_IN, stdin_remote_info_cb, agent);
  std::cout << "> ";
  sleep(10);
   write(STDIN_FILENO, mymap.begin()->first.c_str(), mymap.begin()->first.size());
  fflush (stdout);
}

static gboolean
stdin_remote_info_cb (GIOChannel *source, GIOCondition cond,
    gpointer data)
{
  NiceAgent *agent = reinterpret_cast<NiceAgent*>(data);
  gchar *line = NULL;
  int rval;
  gboolean ret = TRUE;

  if (g_io_channel_read_line (source, &line, NULL, NULL, NULL) ==
      G_IO_STATUS_NORMAL) {

    // Parse remote candidate list and set it on the agent
    rval = parse_remote_data(agent, stream_id, 1, line);
    if (rval == EXIT_SUCCESS) {
      // Return FALSE so we stop listening to stdin since we parsed the
      // candidates correctly
      ret = FALSE;
      g_debug("waiting for state READY or FAILED signal...");
    } else {
      fprintf(stderr, "ERROR: failed to parse remote data\n");
      std::cout << "Enter remote data (single line, no wrapping):\n";
      std::cout << "> ";
      fflush (stdout);
    }
    g_free (line);
  }

  return ret;
}

static void
cb_component_state_changed(NiceAgent *agent, guint _stream_id,
    guint component_id, guint state,
    gpointer data)
{

  g_debug("SIGNAL: state changed %d %d %s[%d]\n",
      _stream_id, component_id, state_name[state], state);

  if (state == NICE_COMPONENT_STATE_CONNECTED) {
    NiceCandidate *local, *remote;

    // Get current selected candidate pair and print IP address used
    if (nice_agent_get_selected_pair (agent, _stream_id, component_id,
                &local, &remote)) {
      gchar ipaddr[INET6_ADDRSTRLEN];

      nice_address_to_string(&local->addr, ipaddr);
      printf("\nNegotiation complete: ([%s]:%d,",
          ipaddr, nice_address_get_port(&local->addr));
      nice_address_to_string(&remote->addr, ipaddr);
      printf(" [%s]:%d)\n", ipaddr, nice_address_get_port(&remote->addr));
    }

    // Listen to stdin and send data written to it
    std::cout << "\nSend lines to remote (Ctrl-D to quit):\n";
    g_io_add_watch(io_stdin, G_IO_IN, stdin_send_data_cb, agent);
    std::cout << "> ";
    fflush (stdout);
  } else if (state == NICE_COMPONENT_STATE_FAILED) {
    g_main_loop_quit (gloop);
  }
}

static gboolean
stdin_send_data_cb (GIOChannel *source, GIOCondition cond,
    gpointer data)
{
  NiceAgent *agent = reinterpret_cast<NiceAgent*>(data);
  gchar *line = NULL;

  if (g_io_channel_read_line (source, &line, NULL, NULL, NULL) ==
      G_IO_STATUS_NORMAL) {
    nice_agent_send(agent, stream_id, 1, strlen(line), line);
    g_free (line);
    std::cout << "> ";
    fflush (stdout);
  } else {
    nice_agent_send(agent, stream_id, 1, 1, "\0");
    // Ctrl-D was pressed.
    g_main_loop_quit (gloop);
  }

  return TRUE;
}

static void
cb_new_selected_pair(NiceAgent *agent, guint _stream_id,
    guint component_id, gchar *lfoundation,
    gchar *rfoundation, gpointer data)
{
  g_debug("SIGNAL: selected pair %s %s", lfoundation, rfoundation);
}

static void
cb_nice_recv(NiceAgent *agent, guint _stream_id, guint component_id,
    guint len, gchar *buf, gpointer data)
{
  if (len == 1 && buf[0] == '\0')
    g_main_loop_quit (gloop);
  printf("%.*s", len, buf);
  fflush(stdout);
}

static NiceCandidate *
parse_candidate(char *scand, guint _stream_id)
{
  NiceCandidate *cand = NULL;
  NiceCandidateType ntype = NICE_CANDIDATE_TYPE_HOST;
  gchar **tokens = NULL;
  guint i;

  tokens = g_strsplit (scand, ",", 5);
  for (i = 0; tokens[i]; i++);
  if (i != 5)
    goto end;

  for (i = 0; i < G_N_ELEMENTS (candidate_type_name); i++) {
    if (strcmp(tokens[4], candidate_type_name[i]) == 0) {
      ntype = static_cast<NiceCandidateType>(i);
      break;
    }
  }
  if (i == G_N_ELEMENTS (candidate_type_name))
    goto end;

  cand = nice_candidate_new(ntype);
  cand->component_id = 1;
  cand->stream_id = _stream_id;
  cand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
  strncpy(cand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION - 1);
  cand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
  cand->priority = atoi (tokens[1]);

  if (!nice_address_set_from_string(&cand->addr, tokens[2])) {
    g_message("failed to parse addr: %s", tokens[2]);
    nice_candidate_free(cand);
    cand = NULL;
    goto end;
  }

  nice_address_set_port(&cand->addr, atoi (tokens[3]));

 end:
  g_strfreev(tokens);

  return cand;
}


std::map<std::string, std::pair<std::string,std::string>>
print_local_data (NiceAgent *agent, guint _stream_id, guint component_id)
{
  std::map<std::string, std::pair<std::string,std::string>> mapdata;
  std::string data = "";
  int result = EXIT_FAILURE;
  gchar *local_ufrag = NULL;
  gchar *local_password = NULL;
  gchar ipaddr[INET6_ADDRSTRLEN];
  GSList *cands = NULL, *item;

  if (!nice_agent_get_local_credentials(agent, _stream_id,
      &local_ufrag, &local_password))
    goto end;

  cands = nice_agent_get_local_candidates(agent, _stream_id, component_id);
  if (cands == NULL)
    goto end;

  data += std::string(local_ufrag) + std::string(local_password);
  //printf("%s %s", local_ufrag, local_password);

  for (item = cands; item; item = item->next) {
    NiceCandidate *c = (NiceCandidate *)item->data;

    nice_address_to_string(&c->addr, ipaddr);

    // (foundation),(prio),(addr),(port),(type)
    // printf(" %s,%u,%s,%u,%s",
    data += c->foundation;
    data += c->priority;
    data += ipaddr;
    data += nice_address_get_port(&c->addr);
    data += candidate_type_name[c->type];
  }
   mapdata = send_connection_string(data);
  std::cout << std::endl;
  result = EXIT_SUCCESS;

end:
  if (local_ufrag)
    g_free(local_ufrag);
  if (local_password)
    g_free(local_password);
  if (cands)
    g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);

  return mapdata;
}


static int
parse_remote_data(NiceAgent *agent, guint _stream_id,
    guint component_id, char *line)
{
  GSList *remote_candidates = NULL;
  gchar **line_argv = NULL;
  const gchar *ufrag = NULL;
  const gchar *passwd = NULL;
  int result = EXIT_FAILURE;
  int i;

  line_argv = g_strsplit_set (line, " \t\n", 0);
  for (i = 0; line_argv && line_argv[i]; i++) {
    if (strlen (line_argv[i]) == 0)
      continue;

    // first two args are remote ufrag and password
    if (!ufrag) {
      ufrag = line_argv[i];
    } else if (!passwd) {
      passwd = line_argv[i];
    } else {
      // Remaining args are serialized canidates (at least one is required)
      NiceCandidate *c = parse_candidate(line_argv[i], _stream_id);

      if (c == NULL) {
        g_message("failed to parse candidate: %s", line_argv[i]);
        goto end;
      }
      remote_candidates = g_slist_prepend(remote_candidates, c);
    }
  }
  if (ufrag == NULL || passwd == NULL || remote_candidates == NULL) {
    g_message("line must have at least ufrag, password, and one candidate");
    goto end;
  }

  if (!nice_agent_set_remote_credentials(agent, _stream_id, ufrag, passwd)) {
    g_message("failed to set remote credentials");
    goto end;
  }

  // Note: this will trigger the start of negotiation.
  if (nice_agent_set_remote_candidates(agent, _stream_id, component_id,
      remote_candidates) < 1) {
    g_message("failed to set remote candidates");
    goto end;
  }

  result = EXIT_SUCCESS;

 end:
  if (line_argv != NULL)
    g_strfreev(line_argv);
  if (remote_candidates != NULL)
    g_slist_free_full(remote_candidates, (GDestroyNotify)&nice_candidate_free);

  return result;
}
