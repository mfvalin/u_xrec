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

#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/RowColumn.h>

#include <xinit.h>
#include <c_wgl.h>
#include <rpnmacros.h>
#include <rec.h>

extern SuperWidgetStruct SuperWidget;
extern _XContour xc;
extern int facteurLissage;

Widget pattTopLevel = NULL;
Widget pattForme, pattFormeChamps, pattFormeAttributs, pattFrameChamps, pattFrameAttributs, pattRc, pattAfficher, pattOk;
Widget pattFrameFonte,pattRCFonte,pattLabelFonte;
Widget pattPanneauLissage,pattFrameLissage,pattOptionsLissage,pattOptionsLissageItems[7];
Widget pattPanneauInterp,pattFrameInterp,pattOptionsInterp,pattOptionsInterpItems[3];
Widget pattPanneauFontSizeLegend,pattOptionsFontSizeLegend,pattOptionsFontSizeLegendItems[5];
Widget pattPanneauFontSizeLabels,pattOptionsFontSizeLabels,pattOptionsFontSizeLabelItems[5];
Widget pattPanneauFontSizeColorLegend,pattOptionsFontSizeColorLegend,pattOptionsFontSizeColorLegendItems[5];
Widget pattChamps[6];

static char *nomPanneauAttributs[] = {"PanneauAttributs", "AttributesPanel"};
static char *labelTopLevel[] = {"Attributs", "Attributes"};
static char *labelOk[] = {"Fermer", "Close"};
static char *labelAfficher[] = {"Redessiner", "Refresh"};
static char *labelFonte[] = {"Taille police","Font Size"};

static char *labelLissage[] =             {"Niveau de lissage    ", "Smoothing factor   "};
static char *labelInterp[] =              {"Degre d'interpolation", "Interpolation Level"};
static char *labelFontSizeLegend[] =      {"Legende              ", "Legend             "};
static char *labelFontSizeLabels[] =      {"Labels               ", "Labels             "};
static char *labelFontSizeColorLegend[] = {"Legende couleurs     ", "Color Table        "};
    
static char *pattLabelOptionsLissage[][7] = {{"Automatique         ", "2", "4", "6", "8", "10", "Maximum"}, 
					      {"Automatic          ", "2", "4", "6", "8", "10", "Maximum"}};
static char *pattLabelOptionsInterp[][3] = {{"Voisin              ", "Bi-lineaire", "Bi-cubique"}, 
					      {"Nearest neighbor    ", "Bilinear", "Bicubic"}};
static char *pattLabelOptionsFontSize[][5] = {{"12                  ", "14", "17", "18", "24"}, 
					      { "12                  ", "14", "17", "18", "24"}};

int fontSizeLegend = 12;
int fontSizeColorLegend = 12;
int fontSizeLabels = 12;
int currentItem;
char panneauAttributsGeometrie[32];

int pattSelectionTerminee;


void SetLissageToggle (w, client_data, call_data) 
Widget	w;		/*  widget id		*/
caddr_t	client_data;	/*  data from application   */
caddr_t	call_data;	/*  data from widget class  */
{
   char *str;
   int lng;

   lng = c_getulng();

   str = XtName(w);
   if (0 == strcmp(str, pattLabelOptionsLissage[lng][0]))
      {
      facteurLissage = -32767;
      }
   else
      {
      if (0 == strcmp(str, pattLabelOptionsLissage[lng][6]))
	 {
	 facteurLissage = 32767;
	 }
      else
	 {
	 facteurLissage = atoi(XtName(w));
	 }
      }
   }

void SetInterpolationToggle (w, client_data, call_data) 
Widget	w;		/*  widget id		*/
caddr_t	client_data;	/*  data from application   */
caddr_t	call_data;	/*  data from widget class  */
{
   char *str;
   int lng;
   int facteurInterp,vrai;
   char interp[8];

   lng = c_getulng();

   str = XtName(w);
   if (0 == strcmp(str, pattLabelOptionsInterp[lng][0]))
      {
      c_ezsetopt("interp_degree","nearest");
      }
   else
      {
      if (0 == strcmp(str, pattLabelOptionsInterp[lng][1]))
	 {
      c_ezsetopt("interp_degree","linear");
	 }
      else
	 {
      c_ezsetopt("interp_degree","cubic");
	 }
      }
   }

void SetFontSizeLegendToggle (w, client_data, call_data) 
Widget	w;		/*  widget id		*/
caddr_t	client_data;	/*  data from application   */
caddr_t	call_data;	/*  data from widget class  */
{
   fontSizeLegend = atoi(XtName(w));
   }

void SetFontSizeColorLegendToggle (w, client_data, call_data) 
Widget	w;		/*  widget id		*/
caddr_t	client_data;	/*  data from application   */
caddr_t	call_data;	/*  data from widget class  */
{
   fontSizeColorLegend = atoi(XtName(w));
   }

void SetFontSizeLabelToggle (w, client_data, call_data) 
Widget	w;		/*  widget id		*/
caddr_t	client_data;	/*  data from application   */
caddr_t	call_data;	/*  data from widget class  */
{
   fontSizeLabels = atoi(XtName(w));
   }

static XtCallbackProc PattOk(w, unused1, unused2)
Widget w;
caddr_t unused1, unused2;
{
   pattSelectionTerminee = TRUE;
   DesactiverPanneauAttributs();
   }

static XtCallbackProc PattAfficher(w, unused1, unused2)
Widget w;
caddr_t unused1, unused2;
{
   int i;

   RedessinerFenetres();

   }


InitPanneauAttributs()
{

   int i;
   Arg args[6];
   XmString string;

   int n,lng;
   char nomShell[128];
   Pixel indCouleurs[16];
   Colormap cmap;

   Xinit("xregarder");
   lng = c_getulng();

   i = 0;
   strcpy(nomShell, XtName(SuperWidget.topLevel));
   strcat(nomShell, nomPanneauAttributs[lng]);
   i = 0;

   if (0 < strlen(panneauAttributsGeometrie)) 
      {
      XtSetArg(args[i],XmNgeometry,panneauAttributsGeometrie);
      i++;
      }
      
   pattTopLevel = XtAppCreateShell(nomShell, nomShell, 
                                   applicationShellWidgetClass,
                                   XtDisplay(SuperWidget.topLevel), args, i);
   i = 0;
   pattForme = (Widget) XmCreateForm(pattTopLevel, "form", args, i);
   XtManageChild(pattForme);

   i = 0;
   XtSetArg(args[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(args[i], XmNrightAttachment, XmATTACH_FORM); i++;
   pattOk = (Widget)XmCreatePushButton(pattForme, labelOk[lng], args, i);
   XtAddCallback(pattOk, XmNactivateCallback, (XtCallbackProc) PattOk, NULL);
   XtManageChild(pattOk);

   i = 0;
   XtSetArg(args[i], XmNrightAttachment, XmATTACH_WIDGET); i++;
   XtSetArg(args[i], XmNrightWidget, pattOk); i++;
   XtSetArg(args[i], XmNtopAttachment, XmATTACH_FORM); i++;
   pattAfficher = (Widget)XmCreatePushButton(pattForme, labelAfficher[lng], args, i);
   XtAddCallback(pattAfficher, XmNactivateCallback, (XtCallbackProc)  PattAfficher, NULL);
   XtManageChild(pattAfficher);

   i = 0;
   XtSetArg(args[i], XmNtopAttachment, XmATTACH_WIDGET); i++;
   XtSetArg(args[i], XmNtopWidget, pattOk); i++;
   XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM); i++;
   pattFormeAttributs = (Widget) XmCreateForm(pattForme, "form", args, i);
   XtManageChild(pattFormeAttributs);

   i = 0;
   pattFrameAttributs = (Widget) XmCreateFrame(pattFormeAttributs, "form", args, i);
   XtManageChild(pattFrameAttributs);

/* Create RowColumn in pattTopLevel */

  i = 0;
   XtSetArg(args[i], XmNnumColumns, 1); i++;
   XtSetArg(args[i], XmNorientation, XmVERTICAL); i++;
   XtSetArg(args[i], XmNpacking, XmPACK_TIGHT); i++;
   pattRc = XmCreateRowColumn(pattFrameAttributs, "pattRc", args, i);
   XtManageChild(pattRc);


   i = 0;
   pattFrameLissage = (Widget) XmCreateFrame(pattRc, "form", args, i);
   XtManageChild(pattFrameLissage);

   i = 0;
   pattOptionsLissage = (Widget)XmCreatePulldownMenu(pattFrameLissage, labelLissage[lng], args, i);

   for (n=0; n < XtNumber(pattLabelOptionsLissage[lng]); n++)
	{
	i = 0;
	string = XmStringCreateLtoR(pattLabelOptionsLissage[lng][n], XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[i], XmNlabelString, string); i++;
	pattOptionsLissageItems[n] = XmCreatePushButtonGadget(pattOptionsLissage, pattLabelOptionsLissage[lng][n], args, i);
	XmStringFree(string);   
	XtAddCallback(pattOptionsLissageItems[n], XmNactivateCallback, (XtCallbackProc)  SetLissageToggle, (XtPointer) n);
	}

   XtManageChildren(pattOptionsLissageItems, XtNumber(pattLabelOptionsLissage[lng]));

   switch(facteurLissage)
      {
      case 32767:
      currentItem = 6;
      break;

      case -32767:
      currentItem = 0;
      break;
      
      case 2:
      currentItem = 1;
      break;

      case 4:
      currentItem = 2;
      break;
      
      case 6:
      currentItem = 3;
      break;
      
      case 8:
      currentItem = 4;
      break;
      
      case 10:
      currentItem = 5;
      break;
      }

   i = 0;
   string = XmStringCreateLtoR(labelLissage[lng], XmSTRING_DEFAULT_CHARSET); 
   XtSetArg(args[i], XmNlabelString, string); i++;
   XtSetArg(args[i], XmNsubMenuId, pattOptionsLissage); i++;
   XtSetArg(args[i], XmNmenuHistory, pattOptionsLissageItems[currentItem]); i++;
   XtSetArg(args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
   pattPanneauLissage = XmCreateOptionMenu(pattFrameLissage, "option_menu1", args, i);
   XmStringFree(string);   

   XtManageChild(pattPanneauLissage);
   
   /*
   ** 
   **
   */
   
   i = 0;
   pattFrameInterp = (Widget) XmCreateFrame(pattRc, "frame", args, i);
   XtManageChild(pattFrameInterp);
   
   pattOptionsInterp = (Widget)XmCreatePulldownMenu(pattFrameInterp, labelInterp[lng], NULL, 0);

   for (n=0; n < XtNumber(pattLabelOptionsInterp[lng][n]); n++)
	{
	i = 0;
	string = XmStringCreateLtoR(pattLabelOptionsInterp[lng][n], XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[i], XmNlabelString, string); i++;
	pattOptionsInterpItems[n] = XmCreatePushButtonGadget(pattOptionsInterp, pattLabelOptionsInterp[lng][n], args, i);
	XmStringFree(string);   
	XtAddCallback(pattOptionsInterpItems[n], XmNactivateCallback, (XtCallbackProc)  SetInterpolationToggle, (XtPointer) n);
	}

   XtManageChildren(pattOptionsInterpItems, XtNumber(pattLabelOptionsInterp[lng]));

   currentItem = 1;

   i = 0;
   string = XmStringCreateLtoR(labelInterp[lng], XmSTRING_DEFAULT_CHARSET); 
   XtSetArg(args[i], XmNlabelString, string); i++;
   XtSetArg(args[i], XmNsubMenuId, pattOptionsInterp); i++;
   XtSetArg(args[i], XmNmenuHistory, pattOptionsInterpItems[currentItem]); i++;
   XtSetArg(args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
   pattPanneauInterp = XmCreateOptionMenu(pattFrameInterp, "option_menu1", args, i);
   XmStringFree(string);   

   XtManageChild(pattPanneauInterp);


/**
**
**/

   i = 0;
   pattFrameFonte = (Widget) XmCreateFrame(pattRc, "frame", args, i);
   XtManageChild(pattFrameFonte);
   
   i = 0;
   XtSetArg(args[i], XmNorientation, XmVERTICAL); i++;
   XtSetArg(args[i], XmNpacking, XmPACK_TIGHT); i++;
   pattRCFonte = XmCreateRowColumn(pattFrameFonte, "pattRc", args, i);
   XtManageChild(pattRCFonte);
   
   pattLabelFonte = (Widget) XmCreateLabel(pattRCFonte, labelFonte[lng], NULL, 0);
   XtManageChild(pattLabelFonte);

   pattOptionsFontSizeLabels = (Widget)XmCreatePulldownMenu(pattRCFonte, labelFontSizeLabels[lng], NULL, 0);

   for (n=0; n < XtNumber(pattLabelOptionsFontSize[lng]); n++)
	{
	i = 0;
	string = XmStringCreateLtoR(pattLabelOptionsFontSize[lng][n], XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[i], XmNlabelString, string); i++;
	pattOptionsFontSizeLabelItems[n] = XmCreatePushButtonGadget(pattOptionsFontSizeLabels, pattLabelOptionsFontSize[lng][n], args, i);
	XmStringFree(string);   
	XtAddCallback(pattOptionsFontSizeLabelItems[n], XmNactivateCallback, (XtCallbackProc)  SetFontSizeLabelToggle, (XtPointer) n);
	}

   XtManageChildren(pattOptionsFontSizeLabelItems, XtNumber(pattLabelOptionsFontSize[lng]));

   switch(fontSizeLabels)
      {
      case 12:
      currentItem = 0;
      break;

      case 14:
      currentItem = 1;
      break;
      
      case 17:
      currentItem = 2;
      break;
      
      case 18:
      currentItem = 3;
      break;
      
      case 24:
      currentItem = 4;
      break;
      }

   i = 0;
   string = XmStringCreateLtoR(labelFontSizeLabels[lng], XmSTRING_DEFAULT_CHARSET); 
   XtSetArg(args[i], XmNlabelString, string); i++;
   XtSetArg(args[i], XmNsubMenuId, pattOptionsFontSizeLabels); i++;
   XtSetArg(args[i], XmNmenuHistory, pattOptionsFontSizeLabelItems[currentItem]); i++;
   XtSetArg(args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
   pattPanneauFontSizeLabels = XmCreateOptionMenu(pattRCFonte, "option_menu1", args, i);
   XmStringFree(string);   

   XtManageChild(pattPanneauFontSizeLabels);

/**
**
**/

   pattOptionsFontSizeColorLegend = (Widget)XmCreatePulldownMenu(pattRCFonte, labelFontSizeColorLegend[lng], NULL, 0);

   for (n=0; n < XtNumber(pattLabelOptionsFontSize[lng]); n++)
	{
	i = 0;
	string = XmStringCreateLtoR(pattLabelOptionsFontSize[lng][n], XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[i], XmNlabelString, string); i++;
	pattOptionsFontSizeColorLegendItems[n] = XmCreatePushButtonGadget(pattOptionsFontSizeColorLegend, pattLabelOptionsFontSize[lng][n], args, i);
	XmStringFree(string);   
	XtAddCallback(pattOptionsFontSizeColorLegendItems[n], XmNactivateCallback, (XtCallbackProc)  SetFontSizeColorLegendToggle, (XtPointer) n);
	}

   XtManageChildren(pattOptionsFontSizeColorLegendItems, XtNumber(pattLabelOptionsFontSize[lng]));

   switch(fontSizeColorLegend)
      {
      case 12:
      currentItem = 0;
      break;

      case 14:
      currentItem = 1;
      break;
      
      case 17:
      currentItem = 2;
      break;
      
      case 18:
      currentItem = 3;
      break;
      
      case 24:
      currentItem = 4;
      break;
      }

   i = 0;
   string = XmStringCreateLtoR(labelFontSizeColorLegend[lng], XmSTRING_DEFAULT_CHARSET); 
   XtSetArg(args[i], XmNlabelString, string); i++;
   XtSetArg(args[i], XmNsubMenuId, pattOptionsFontSizeColorLegend); i++;
   XtSetArg(args[i], XmNmenuHistory, pattOptionsFontSizeColorLegendItems[currentItem]); i++;
   XtSetArg(args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
   pattPanneauFontSizeColorLegend = XmCreateOptionMenu(pattRCFonte, "option_menu1", args, i);
   XmStringFree(string);   

   XtManageChild(pattPanneauFontSizeColorLegend);

/**
**
**/

   pattOptionsFontSizeLegend = (Widget)XmCreatePulldownMenu(pattRCFonte, labelFontSizeLegend[lng], NULL, 0);

   for (n=0; n < XtNumber(pattLabelOptionsFontSize[lng]); n++)
	{
	i = 0;
	string = XmStringCreateLtoR(pattLabelOptionsFontSize[lng][n], XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[i], XmNlabelString, string); i++;
	pattOptionsFontSizeLegendItems[n] = XmCreatePushButtonGadget(pattOptionsFontSizeLegend, pattLabelOptionsFontSize[lng][n], args, i);
	XmStringFree(string);   
	XtAddCallback(pattOptionsFontSizeLegendItems[n], XmNactivateCallback, (XtCallbackProc)  SetFontSizeLegendToggle, (XtPointer) n);
	}

   XtManageChildren(pattOptionsFontSizeLegendItems, XtNumber(pattLabelOptionsFontSize[lng]));

   switch(fontSizeLegend)
      {
      case 12:
      currentItem = 0;
      break;

      case 14:
      currentItem = 1;
      break;
      
      case 17:
      currentItem = 2;
      break;
      
      case 18:
      currentItem = 3;
      break;
      
      case 24:
      currentItem = 4;
      break;
      }

   i = 0;
   string = XmStringCreateLtoR(labelFontSizeLegend[lng], XmSTRING_DEFAULT_CHARSET); 
   XtSetArg(args[i], XmNlabelString, string); i++;
   XtSetArg(args[i], XmNsubMenuId, pattOptionsFontSizeLegend); i++;
   XtSetArg(args[i], XmNmenuHistory, pattOptionsFontSizeLegendItems[currentItem]); i++;
   XtSetArg(args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
   pattPanneauFontSizeLegend = XmCreateOptionMenu(pattRCFonte, "option_menu1", args, i);
   XmStringFree(string);   

   XtManageChild(pattPanneauFontSizeLegend);

   }

ActiverPanneauAttributs()
{

   Colormap cmap;
   Arg args[2];
   int i;

   if (!pattTopLevel)
      InitPanneauAttributs();

   pattSelectionTerminee = False;
   
   if (!XtIsRealized(pattTopLevel))
      {
      XtRealizeWidget(pattTopLevel);
      CheckColormap(pattTopLevel);
      }
      
   f77name(xpattact)();

   }

f77name(xpattact)()
{
   LocalEventLoop(pattTopLevel);
   }


DesactiverPanneauAttributs()
{
   int i;

   XtUnrealizeWidget(pattTopLevel);
   }

AttrMgrGetFontSizeLegend()
{
   return fontSizeLegend;
}

AttrMgrGetFontSizeColorTable()
{
   return fontSizeColorLegend;
}

AttrMgrGetFontSizeLabels()
{
   return fontSizeLabels;
}

f77name(c_satratr)(item,valeur,lenItem,lenValeur)
char item[],valeur[];
{
   Arg args[10];
   int i;
   int indItem;
   char fakeWidgetName[32];

   item[lenItem-1] = '\0';
   valeur[lenValeur-1] = '\0';
   nettoyer(item);
   nettoyer(valeur);

   if (0 == strcmp(item,"geometrie") || 0 == strcmp(item,"geometry"))
      {
      strcpy(panneauAttributsGeometrie,valeur);
      }
      
   if (0 == strcmp(item,"niveau_de_lissage") || 0 == strcmp(item,"smoothing_factor"))
      {
      if (0 == strncmp("automati",valeur,8))
         {
         facteurLissage =-32767;
         }
      else
         {
         if (0 == strcmp(valeur,"maximum"))
            {
            facteurLissage = 32767;
            }
         else
            {
            facteurLissage = atoi(valeur);
            }
         }
      }

   if (0 == strcmp(item,"taille_police_labels") || 0 == strcmp(item,"labels_font_size"))
      {
      fontSizeLabels = atoi(valeur);
      }

   if (0 == strcmp(item,"taille_police_legende_couleurs") || 0 == strcmp(item,"color_legend_font_size"))
      {
      fontSizeColorLegend = atoi(valeur);
      }

   if (0 == strcmp(item,"taille_police_legende") || 0 == strcmp(item,"legend_font_size"))
      {
      fontSizeLegend = atoi(valeur);
      }
}

f77name(c_sanmatr)(item,valeur,lenItem,lenValeur)
{
}


f77name(c_scpeatr)(item,valeur,lenItem,lenValeur)
{
}


EcrAtrAtr(fichierDemarrage)
FILE *fichierDemarrage;
{
   char tableau[32];
   char ligne[80];
   char item[32],valeur[32];
   int i;

   Arg  args[10];
   XmString label;
   XmFontList fontListe;
   char *geom;
   Window root;
   Position x,y;
   Display *disp;
   Window win;

   strcpy(tableau, "attributs_divers");
   
   strcpy(item,"geometrie");
   if (pattTopLevel)
      {
      disp = XtDisplay(pattTopLevel);
      win  = XtWindow(pattTopLevel);
      i = 0;
      XtSetArg(args[i], XmNx, &x); i++;
      XtSetArg(args[i], XmNy, &y); i++;
      XtGetValues(pattTopLevel, args, i);
      
      sprintf(valeur,"%+d%+d",x,y);
      sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
      fprintf(fichierDemarrage,"%s\n",ligne);
      }
   else
      {
      if (strlen(panneauAttributsGeometrie) > 0)
         {
         strcpy(valeur,panneauAttributsGeometrie);
         sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
         fprintf(fichierDemarrage,"%s\n",ligne);
         }
      }


   strcpy(item, "niveau_de_lissage");
   switch (facteurLissage)
      {
      case -32767:
      strcpy(valeur, "automatique");
      break;

      case 32767:
      strcpy(valeur, "maximum");
      break;
      
      default:
      sprintf(valeur, "%2d", facteurLissage);
      break;
      }
   sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
   fprintf(fichierDemarrage,"%s\n",ligne);

   strcpy(item, "taille_police_labels");
   sprintf(valeur, "%2d", fontSizeLabels);
   sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
   fprintf(fichierDemarrage,"%s\n",ligne);
   
   strcpy(item, "taille_police_legende");
   sprintf(valeur, "%2d", fontSizeLegend);
   sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
   fprintf(fichierDemarrage,"%s\n",ligne);

   strcpy(item, "taille_police_legende_couleurs");
   sprintf(valeur, "%2d", fontSizeColorLegend);
   sprintf(ligne, " setitem('%s','%s','%s')",tableau,item,valeur);
   fprintf(fichierDemarrage,"%s\n",ligne);

   }
