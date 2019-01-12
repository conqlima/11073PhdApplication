/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file plugin_tcp.h
 * \brief TCP plugin header.
 *
 * Copyright (C) 2010 Signove Tecnologia Corporation.
 * All rights reserved.
 * Contact: Signove Tecnologia Corporation (contact@signove.com)
 *
 * $LICENSE_TEXT:BEGIN$
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and appearing
 * in the file LICENSE included in the packaging of this file; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * $LICENSE_TEXT:END$
 *
 * \author Adrian Guedes
 * \author Fabricio Silva Epaminondas
 * \date Jul 05, 2010
 */


#ifndef _M_PLUGIN_CASTALIA_H_
#define _M_PLUGIN_CASTALIA_H_

extern "C" {
#include "communication/plugin/plugin.h"
}

int m_plugin_network_castalia_manager_setup(CommunicationPlugin *plugin, int m_port);
int m_getControlPacketsReceived(int addr);
int m_getControlPacketsSent(int addr);
int m_getMeasurementPacketsTotal(int addr);
void m_clearVarMap(void);

#endif /*_M_PLUGIN_CASTALIA_H_*/
