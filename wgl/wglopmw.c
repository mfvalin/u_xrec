/* RMNLIB - Library of useful routines for C and FORTRAN programming
 * Copyright (C) 1975-2001  Division de Recherche en Prevision Numerique
 *                          Environnement Canada
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>

#include <wgl.h>

int f77name(wglopmw)(char nomFenetre[], int *wglWinID, int lenNomFenetre)
{
   char nomTemp[256];
   int  winid;

   strncpy(nomTemp, nomFenetre, lenNomFenetre);
   nomTemp[lenNomFenetre] = '\0';
   
   winid = c_wglopmw(nomTemp, *wglWinID);
   return winid;
   }

/**
******
**/

int c_wglopmw(char *nomFenetre, int wglWinID)
{
  wglc_wgl->wglopmw(nomFenetre, wglWinID);
}
