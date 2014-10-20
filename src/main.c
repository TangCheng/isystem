/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2014 TangCheng <tangcheng2005@gmail.com>
 * 
 * isystem is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * isystem is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "isystem.h"
int main()
{
	IpcamISystem *isystem = g_object_new(IPCAM_ISYSTEM_TYPE, "name", "isystem", NULL);
	ipcam_base_service_start(IPCAM_BASE_SERVICE(isystem));
	return (0);
}

