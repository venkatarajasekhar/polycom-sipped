/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author : Richard GAYRAUD - 04 Nov 2003
 *           Olivier Jacques
 *           From Hewlett Packard Company.
 *           Shriram Natarajan
 *           Peter Higginson
 *           Eric Miller
 *           Venkatesh
 *           Enrico Hartung
 *           Nasir Khan
 *           Lee Ballard
 *           Guillaume Teissier from FTR&D
 *           Wolfgang Beck
 *           Venkatesh
 *           Vlad Troyanker
 *           Charles P Wright from IBM Research
 *           Amit On from Followap
 *           Jan Andres from Freenet
 *           Ben Evans from Open Cloud
 *           Marc Van Diest from Belgacom
 *           Michael Dwyer from Cibation
 *           Polycom Inc. (Edward Estabrook, Richard Lum).  Contributions (c) 2010 - 2013
 */


#include "common.hpp"
#include "deadcall.hpp"
#include "logging.hpp"
#include "screen.hpp"
#include "sipp_globals.hpp"
#include "stat.hpp"
//
#include <iterator>
#include <algorithm>
#include <fstream>
#include <iostream>
#ifdef PCAPPLAY
#include "send_packets.hpp"
#endif

deadcall::deadcall(char *id, const char *reason) : listener(id, true)
{
  this->expiration = clock_tick + deadcall_wait;
  this->reason = strdup(reason);
  setPaused();
}

deadcall::~deadcall()
{
  free(reason);
}

bool deadcall::process_incoming(char * msg, struct sockaddr_storage *src, struct sipp_socket *socket)
{
  char buffer[MAX_HEADER_LEN];
  DEBUGIN();

  CStat::globalStat(CStat::E_DEAD_CALL_MSGS);

  setRunning();

  snprintf(buffer, MAX_HEADER_LEN, "Dead call %s (%s)", id, reason);

  WARNING("%s, received '%s'", buffer, msg);

  TRACE_MSG("-----------------------------------------------\n"
            "Dead call %s recieved a %s message:\n\n%s\n",
            id, TRANSPORT_TO_STRING(transport), msg);

  expiration = clock_tick + deadcall_wait;

  DEBUGOUT();
  return run();
}

bool deadcall::process_twinSippCom(char * msg)
{
  CStat::globalStat(CStat::E_DEAD_CALL_MSGS);
  TRACE_MSG("Received twin message for dead (%s) call %s:%s\n", reason, id, msg);
  return true;
}

bool deadcall::run()
{
  if (clock_tick > expiration) {
    delete this;
    return false;
  } else {
    setPaused();
    return true;
  }
}

unsigned int deadcall::wake()
{
  return expiration;
}

/* Dump call info to error log. */
void deadcall::dump()
{
  WARNING("%s: Dead Call (%s) expiring at %lu", id, reason, expiration);
}
