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

#include <string.h>
#include <fcntl.h>
#include <rec.h>
#include <gmp.h>
#include <xinit.h>
#include <wgl.h>
#include <rpnmacros.h>
#include <limits.h>
/***************************/
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
   
#ifndef HP
long times (struct tms *buffer);
#else
extern clock_t times();
#endif

struct tms buffer_temps;
extern SuperWidgetStruct SuperWidget;
extern Widget            wglTopLevel;
extern _XContour xc;
extern _AnimInfo animInfo;
extern GeoMapInfoStruct mapInfo;


extern Display *wglDisp;
extern int  wglScrNum;
extern Window  wglWin, wglDrawable;
extern GC wglLineGC;

extern void EffacerFenetreAffichage();

static char *pasAssezDeMemoire[] = {"Quantite de memoire insuffisante.\nSeulement %4d images sur %4d peuvent etre chargees.\n\
L'animation serait possible si les dimensions de la fenetre\netaient reduites a %4d x %4d",
                                     "Not enough memory.\nOnly %4d images on %4d can be loaded.\nHowever, animation could be possible if window dimension is reduced to %4d x4d"};

static char *pasAssezDeNiveaux[] = {"Nombre insuffisant de niveaux.",
                                       "Insufficient number of levels."};

extern int PasAssezDeMemoire();
extern int interpolationVerticale;
static int nbNiveauxCoupe;
static char messageErreur[512];
static int lng;

void VAnimMgrInterpolateVerticalField(_Champ *champ, float niveau);

VAnimerFrames(int nbFrames)
{
  if (interpolationVerticale)
    {
    VAnimerFramesAvecInterpolation(nbFrames);
    }
  else
    {
    VAnimerFramesSansInterpolation(nbFrames);
    }
}

VAnimerFramesAvecInterpolation(nbFrames)
int nbFrames;
{
   int i, j, k, ier, ii;
   int n;
   int annulationDemandee;
   int statut, signeIncrement;
   float incrementVertical;
   int res, fenetreAffichage, status;
   int nbChampsActifs;
   _Champ *champ;

   int temps1, temps2;
   int largeurFenetre, hauteurFenetre, nouvelleLargeur, nouvelleHauteur;
   static int lastLargeur, lastHauteur;
   int animationContinue;
   int op,i_initial;
   double dt,dt1,dt2;
   int datev;
   double weight;
   float *fld, *fld1, *fld2, niveauCourant;
   double delai;

   nbChampsActifs = FldMgrGetNbChampsActifs();
   
   if (nbChampsActifs == 0)
     {
     return;
     }
   
   lng = c_getulng();
   GetFenetreAffichageID(&fenetreAffichage);
   c_wglsetw(fenetreAffichage);
   
   xc.annulationDemandee = False;
   xc.flagInterrupt = False;
   FldMgrGetChamp(&champ, 0);
   
   if ((*champ).champModifie)
     {
     RefuserOperation();
     return;
     }
   
   c_wglsetw(fenetreAffichage);
   res = FldMgrLoadVerticalXSection();
   
   if ((*champ).coupe.nbNiveauxCoupe < 2)
     {
     MessageAvertissement(pasAssezDeNiveaux[lng], AVERTISSEMENT);
     return;
     }
   
   if (res == CHARGEMENT_ANNULE)
     {
     AfficherOperationAnnulee();
     AfficherMessageInfoStandard();
     return;
     }
   
   op = CtrlMgrGetMathOp();
   i = (*champ).coupe.indChampCourant;
   
   XSetErrorHandler(PasAssezDeMemoire);
   
   c_wglgwz(&largeurFenetre, &hauteurFenetre);
   c_wglbbf();
   EffacerFenetreAffichage();
   
   if (nbFrames > 0)
     signeIncrement =  1;
   else
     signeIncrement = -1;
   
   if (nbFrames == 2 || nbFrames == -2)
     {
     UnSetCurseur(fenetreAffichage);
     SetIgnoreMode();
     animationContinue = TRUE;
     }
   else
     animationContinue = FALSE;
   
   FlusherTousLesEvenements();
   
   nbNiveauxCoupe = (*champ).coupe.nbNiveauxCoupe;
   niveauCourant = champ->coupe.niveauxCoupe[0];
   incrementVertical = animInfo.intervalle;
   do 
     {
     niveauCourant -= signeIncrement*animInfo.intervalle;
     if (niveauCourant < champ->coupe.niveauxCoupe[nbNiveauxCoupe-1])
       {     
       if (animInfo.typeDefilement == DEFILEMENT_AVANT_ARRIERE)
	 {
	 i = nbNiveauxCoupe-1;
	 niveauCourant = champ->coupe.niveauxCoupe[nbNiveauxCoupe-1];
	 }
       else
	 {
	 i = 0;
	 niveauCourant = champ->coupe.niveauxCoupe[0];
	 }
       }
     i = i % (*champ).coupe.nbNiveauxCoupe;
     if (i < 0) i = (*champ).coupe.nbNiveauxCoupe-1;
     
     for (n=0; n < nbChampsActifs; n++)
       {
       FldMgrGetChamp(&champ, n);
       if (champ->coupe.nbNiveauxCoupe > 0)
	 {
	 k = i;
	 while (niveauCourant <= champ->coupe.niveauxCoupe[k] && k < nbNiveauxCoupe)
	   k++;
	 k--;
	 i = k;
	 (*champ).coupe.indChampCourant = i;
	 (*champ).cle = (*champ).coupe.clesNiveaux[i];
	 (*champ).niveau = niveauCourant;
	 (*champ).ip1 = ROUND(niveauCourant);
	 switch (champ->natureTensorielle)
	   {
	   case SCALAIRE:
	     (*champ).fldmin[op] = (*champ).coupe.FLDmin3d[op];
	     (*champ).fldmax[op] = (*champ).coupe.FLDmax3d[op];
	     break;

	   case VECTEUR:
	     (*champ).uumin[op] = (*champ).coupe.UUmin3d[op];
	     (*champ).uumax[op] = (*champ).coupe.UUmax3d[op];
	     (*champ).vvmin[op] = (*champ).coupe.VVmin3d[op];
	     (*champ).vvmax[op] = (*champ).coupe.VVmax3d[op];
	     (*champ).uvmin[op] = (*champ).coupe.UVmin3d[op];
	     (*champ).uvmax[op] = (*champ).coupe.UVmax3d[op];
	     break;
	   }

	 FldMgrUpdateFldParams(champ);
	 
	 if ((*champ).coupe.clesNiveaux[i] >= 0)
	   {
	   VAnimMgrInterpolateVerticalField(champ, niveauCourant);
	   }
	 }
       }
     
     for (n=0; n < nbChampsActifs; n++)
       {
       AfficherCarte(n);
       }
     
     FlusherTousLesEvenements();
     c_wglswb();
     animInfo.flagsImagesChargees[i] = TRUE;
     
     delai = (double) animInfo.delai;
     f77name(micro_sleep)(&delai);
     FlusherTousLesEvenements();
     
     if (animInfo.typeDefilement == DEFILEMENT_AVANT_ARRIERE)
       {

       if (niveauCourant >= champ->coupe.niveauxCoupe[0] || niveauCourant <= champ->coupe.niveauxCoupe[nbNiveauxCoupe-1])
	 {
	 signeIncrement *= -1;
	 }
       }
     } while (!c_wglanul() && animationContinue);
   
   UnsetIgnoreMode();
   UnSetCurseur(fenetreAffichage);
   VRemettreChampsAJour(i);
   lastLargeur = largeurFenetre;
   lastHauteur = hauteurFenetre;
   c_wglfbf();
}

VAnimerFramesSansInterpolation(nbFrames)
int nbFrames;
{
   int i, j, k, ier, ii;
   int n;
   int annulationDemandee;
   int statut, increment;
   
   int res, fenetreAffichage, status;
   int nbChampsActifs;
   _Champ *champ;

   int npts,temps1, temps2;
   int largeurFenetre, hauteurFenetre, nouvelleLargeur, nouvelleHauteur;
   static int lastLargeur, lastHauteur;
   int animationContinue;
   int op,i_initial;
   double dt,dt1,dt2;
   int datev;
   double weight;
   float *fld, *fld1, *fld2;

   nbChampsActifs = FldMgrGetNbChampsActifs();
   
   if (nbChampsActifs == 0)
     {
     return;
     }
   
   if (nbFrames == 1)
     {
     VAnimerFrame();
     return;
     }

   lng = c_getulng();
   GetFenetreAffichageID(&fenetreAffichage);
   c_wglsetw(fenetreAffichage);
   
   xc.annulationDemandee = False;
   xc.flagInterrupt = False;
   FldMgrGetChamp(&champ, 0);
   
   npts = champ->dst.ni*champ->dst.nj*champ->dst.nk;
   
   if ((*champ).champModifie)
     {
     RefuserOperation();
     return;
     }
   
   c_wglsetw(fenetreAffichage);
   res = FldMgrLoadVerticalXSection();
   if ((*champ).coupe.nbNiveauxCoupe < 2)
     {
     MessageAvertissement(pasAssezDeNiveaux[lng], AVERTISSEMENT);
     return;
     }
   
   if (res == CHARGEMENT_ANNULE)
     {
     AfficherOperationAnnulee();
     AfficherMessageInfoStandard();
     return;
     }
   
   op = CtrlMgrGetMathOp();
   i = (*champ).coupe.indChampCourant;
   
   c_wglgwz(&largeurFenetre, &hauteurFenetre);
   c_wglbbf();
   EffacerFenetreAffichage();
   
   if (nbFrames > 0)
     increment =  1;
   else
     increment = -1;
   
   if (nbFrames == 2 || nbFrames == -2)
     {
     UnSetCurseur(fenetreAffichage);
     SetIgnoreMode();
     animationContinue = TRUE;
     }
   else
     animationContinue = FALSE;
   
   FlusherTousLesEvenements();
   
   nbNiveauxCoupe = (*champ).coupe.nbNiveauxCoupe;
   
   if (animInfo.animationRapide)
     {
     if (!animInfo.imagesDejaAllouees || lastLargeur != largeurFenetre || lastHauteur != hauteurFenetre)
       {
       LibererImages();
       do
	 {
	 } while (VAllouerImages() == PAS_ASSEZ_DE_MEMOIRE);
       }
     }
   
     i = (*champ).coupe.indChampCourant;
     i_initial = i;
   
   do 
     {
     i = i % (*champ).coupe.nbNiveauxCoupe;
     if (i < 0)
       i += (*champ).coupe.nbNiveauxCoupe;
   
     if (animInfo.animationRapide && animInfo.flagsImagesChargees[i])
       {
       XCopyArea(wglDisp, animInfo.pixmaps[i], wglWin, wglLineGC, 0, 0, largeurFenetre, hauteurFenetre, 0, 0);
       }
     else
       {
       for (n=0; n < nbChampsActifs; n++)
	 {
	 FldMgrGetChamp(&champ, n);
	 if ((*champ).coupe.fld3d != NULL)
	   {
	   (*champ).coupe.indChampCourant = i;
	   (*champ).cle = (*champ).coupe.clesNiveaux[i];
	   if ((*champ).cle >= 0)
	     {
	     (*champ).fld = (*champ).coupe.fld3d[i];
	     (*champ).fldmin[op] = (*champ).coupe.FLDmin3d[op];
	     (*champ).fldmax[op] = (*champ).coupe.FLDmax3d[op];
	     
	     FldMgrGetFstPrm(champ);
	     FldMgrUpdateFldParams(champ);
	     }
	   }
	 }
       
       for (n=0; n < nbChampsActifs; n++)
	 {
	 AfficherCarte(n);
	 }
       
       FlusherTousLesEvenements();
       c_wglswb();
       animInfo.flagsImagesChargees[i] = TRUE;
       
       if (animInfo.animationRapide)
	 {
	 XCopyArea(wglDisp, wglWin, animInfo.pixmaps[i], wglLineGC, 0, 0, largeurFenetre, hauteurFenetre, 0, 0);
	 }
       
       temps1 = times(&buffer_temps);
       temps2 = times(&buffer_temps) - temps1;
       while (animInfo.delai > temps2)
	 {
	 temps2 = times(&buffer_temps) - temps1;
	 }
       FlusherTousLesEvenements();
       
       if (animInfo.typeDefilement == DEFILEMENT_AVANT_ARRIERE)
	 {
	 if (i == 0 || i == (animInfo.nbImages-1))
	   {
	   increment *= -1;
	   }
	 }
       }
     i+=increment;
     } while (!c_wglanul() && animationContinue);
   
   UnsetIgnoreMode();
   UnSetCurseur(fenetreAffichage);
   VRemettreChampsAJour(i);
   lastLargeur = largeurFenetre;
   lastHauteur = hauteurFenetre;
   c_wglfbf();
   }

/**
 **************************************************************
 **************************************************************
 **/

VAfficherOperationAnnulee()
{

   static char *operationAnnulee[] = {"\nOperation annulee\n\n", "\nOperation cancelled\n\n"};
   int lng;

   lng = c_getulng();
   
   MessageAvertissement(operationAnnulee[lng], AVERTISSEMENT);

   }

/**
 **************************************************************
 **************************************************************
 **/

VRemettreChampsAJour(i)
int i;
{
   _Champ (*champ);
   int n, nbChampsActifs,op,npts;
   
   op = CtrlMgrGetMathOp();
   nbChampsActifs = FldMgrGetNbChampsActifs();

   FldMgrGetChamp(&champ, 0);
   npts = champ->dst.ni*champ->dst.nj*champ->dst.nk;

   for  (n=0; n < nbChampsActifs; n++)
     {
     FldMgrGetChamp(&champ, n);
     if ((*champ).coupe.fld3d != NULL)
       {
       (*champ).coupe.indChampCourant = i;
       (*champ).cle = (*champ).coupe.clesNiveaux[i];
       if ((*champ).cle >= 0)
	 {
	 switch (champ->natureTensorielle)
	   {
	   case SCALAIRE:
	     (*champ).fld = (*champ).coupe.fld3d[i];
	     (*champ).fldmin[op] = (*champ).coupe.FLDmin3d[op];
	     (*champ).fldmax[op] = (*champ).coupe.FLDmax3d[op];
	     break;

	   case VECTEUR:
	     (*champ).uu = (*champ).coupe.uu3d[i];
	     (*champ).vv = (*champ).coupe.vv3d[i];
	     (*champ).uumin[op] = (*champ).coupe.UUmin3d[op];
	     (*champ).uumax[op] = (*champ).coupe.UUmax3d[op];
	     (*champ).vvmin[op] = (*champ).coupe.UUmin3d[op];
	     (*champ).vvmax[op] = (*champ).coupe.UUmax3d[op];
	     (*champ).uvmin[op] = (*champ).coupe.UVmin3d[op];
	     (*champ).uvmax[op] = (*champ).coupe.UVmax3d[op];
	     break;
	   }
	   
	 FldMgrGetFstPrm(champ);
	 FldMgrUpdateFldParams(champ);
	 }
       }
     }
}

/**
 **************************************************************
 **************************************************************
 **/

VAllouerImages()
{
   int i, j;
   int x, y;
   Window root;
   unsigned int width, height, border_width, depth, nplanes;
   int largeurFenetre, hauteurFenetre;
   int status, nouvelleLargeur, nouvelleHauteur; 
   int wglWin;
   
   c_wglgwz(&largeurFenetre, &hauteurFenetre);
   animInfo.nbImages = nbNiveauxCoupe;
   nplanes = c_wglgpl();
   for (i=0; i < nbNiveauxCoupe; i++)
      {
      animInfo.pixmaps[i] = XCreatePixmap(wglDisp, RootWindow(wglDisp, wglScrNum), largeurFenetre, hauteurFenetre, nplanes);
      status = XGetGeometry(wglDisp, animInfo.pixmaps[i], &root, &x, &y, &width, &height, &border_width, &depth);
      if (status == 0)
	 {
	 nouvelleLargeur = (int)((float)largeurFenetre*sqrt((float)(i)/(float)animInfo.nbImages));
	 nouvelleHauteur = (int)((nouvelleLargeur*hauteurFenetre)/largeurFenetre);
	 nouvelleLargeur = (int)(0.95*nouvelleLargeur);
	 nouvelleHauteur = (int)(0.95*nouvelleHauteur);
	 for (j=0; j <= i; j++)
	    {
	    XFreePixmap(wglDisp, animInfo.pixmaps[j]);
	    animInfo.pixmaps[j] = NULL;
	    animInfo.flagsImagesChargees[j] = NULL;
	    }
	 wglWin = c_wglgacw();
	 XResizeWindow(wglDisp,wglWin,nouvelleLargeur,nouvelleHauteur);
	 c_wglsbf();
	 c_wgldbf();
	 c_wglbbf();
	 return PAS_ASSEZ_DE_MEMOIRE;
	 }
      }

   animInfo.imagesDejaAllouees = TRUE;
   return 0;
   }
   
   
VAnimerFrame()
{
   int i, j, k, ier;
   int n;
   int annulationDemandee;
   int statut, increment;
   
   int res, fenetreAffichage, status;
   int nbChampsActifs;
   _Champ *champ;

   int npts,temps1, temps2;
   int largeurFenetre, hauteurFenetre, nouvelleLargeur, nouvelleHauteur;
   static int lastLargeur, lastHauteur;
   int animationContinue;
   int op,i_initial;

   nbChampsActifs = FldMgrGetNbChampsActifs();

   if (nbChampsActifs == 0)
      {
      return;
      }

   lng = c_getulng();
   GetFenetreAffichageID(&fenetreAffichage);
   c_wglsetw(fenetreAffichage);
   
   xc.annulationDemandee = False;
   xc.flagInterrupt = False;
   FldMgrGetChamp(&champ, 0);

   npts = champ->dst.ni*champ->dst.nj*champ->dst.nk;

   if ((*champ).champModifie)
      {
      RefuserOperation();
      return;
      }

   c_wglsetw(fenetreAffichage);
   res = FldMgrLoadVerticalXSection();
   if ((*champ).coupe.nbNiveauxCoupe < 2)
     {
     MessageAvertissement(pasAssezDeNiveaux[lng], AVERTISSEMENT);
     return;
     }
   
   if (res == CHARGEMENT_ANNULE)
      {
      AfficherOperationAnnulee();
      AfficherMessageInfoStandard();
      return;
      }
   
   i = (*champ).coupe.indChampCourant;
   
   XSetErrorHandler(PasAssezDeMemoire);
   
   c_wglgwz(&largeurFenetre, &hauteurFenetre);
   c_wglbbf();
   EffacerFenetreAffichage();

   increment =  1;

   animationContinue = FALSE;

   FlusherTousLesEvenements();
   
   nbNiveauxCoupe = (*champ).coupe.nbNiveauxCoupe;
   
   i = (*champ).coupe.indChampCourant;
   i_initial = i;

   do 
      {
      i += increment;
      i = i % (*champ).coupe.nbNiveauxCoupe;
      if (i < 0)
	i += (*champ).coupe.nbNiveauxCoupe;
      
      if (animInfo.animationRapide && animInfo.flagsImagesChargees[i])
	 {
	 XCopyArea(wglDisp, animInfo.pixmaps[i], wglWin, wglLineGC, 0, 0, largeurFenetre, hauteurFenetre, 0, 0);
	 }
      else
	 {
	 for (n=0; n < nbChampsActifs; n++)
	    {
	    FldMgrGetChamp(&champ, n);
	    if ((*champ).coupe.fld3d != NULL)
	      {
	      (*champ).coupe.indChampCourant = i;
	      (*champ).cle = (*champ).coupe.clesNiveaux[i];
	      if ((*champ).cle >= 0)
		{
		(*champ).fld = (*champ).coupe.fld3d[i];
		(*champ).fldmin[op] = (*champ).coupe.FLDmin3d[op];
		(*champ).fldmax[op] = (*champ).coupe.FLDmax3d[op];
		
		FldMgrGetFstPrm(champ);
		FldMgrUpdateFldParams(champ);
		}
	      }
	    }
	 }
	 
      for (n=0; n < nbChampsActifs; n++)
	{
	AfficherCarte(n);
	}
      
      FlusherTousLesEvenements();
      c_wglswb();
      
      animInfo.flagsImagesChargees[i] = TRUE;
      
      if (animInfo.animationRapide)
	{
	XCopyArea(wglDisp, wglWin, animInfo.pixmaps[i], wglLineGC, 0, 0, largeurFenetre, hauteurFenetre, 0, 0);
	}
      
      temps1 = times(&buffer_temps);
      temps2 = times(&buffer_temps) - temps1;
      while (animInfo.delai > temps2)
	{
	 temps2 = times(&buffer_temps) - temps1;
	 }
      FlusherTousLesEvenements();
      
      if (animInfo.typeDefilement == DEFILEMENT_AVANT_ARRIERE)
	 {
	 if (i == 0 || i == (animInfo.nbImages-1))
	    {
	    increment *= -1;
	    }
	 }
      } while (!c_wglanul() && animationContinue);
  
   UnsetIgnoreMode();
   UnSetCurseur(fenetreAffichage);
   VRemettreChampsAJour(i);
   lastLargeur = largeurFenetre;
   lastHauteur = hauteurFenetre;
   c_wglfbf();
   }

void VAnimMgrInterpolateVerticalField(_Champ *champ, float niveau)
{
  static int lastIndex = 0;

   int i, j, k, ier, n;
   int annulationDemandee;
   int statut, increment;
   
   int res, fenetreAffichage, status;
   int nbChampsActifs;

   int npts,temps1, temps2;
   int op,i_initial;
   double dt,dt1,dt2;
   double weight;
   float *fld, *fld1, *fld2, *uu1, *uu2, *vv1, *vv2, *uv1, *uv2;
   int ldatev, found,nbNiveauxCoupe;
   static int last_index;

   npts = champ->dst.ni*champ->dst.nj*champ->dst.nk;
   
   nbNiveauxCoupe = (*champ).coupe.nbNiveauxCoupe;
   
   i = 0;
   found = 0;
   while (niveau < champ->coupe.niveauxCoupe[i] && found == 0 && i < nbNiveauxCoupe)
     {
     i++;
     }
   i--;
   if (i < 0) i = 0;
   found = i;
   if (found == champ->coupe.nbNiveauxCoupe) 
     {
     found = 0;
     }

   i = (i < champ->coupe.nbNiveauxCoupe-1 ) ? i : champ->coupe.nbNiveauxCoupe-2;

   weight = (niveau - champ->coupe.niveauxCoupe[i])/(champ->coupe.niveauxCoupe[i+1]-champ->coupe.niveauxCoupe[i]);
   switch (champ->natureTensorielle)
     {
     case SCALAIRE:
       for (n=0; n < champ->dst.ni*champ->dst.nj; n++)
	 {
	 champ->fld[n]=((1.0-weight)*champ->coupe.fld3d[i][n]+weight*champ->coupe.fld3d[i+1][n]);
	 }
       break;

     case VECTEUR:
       for (n=0; n < champ->dst.ni*champ->dst.nj; n++)
	 {
	 champ->uu[n]=((1.0-weight)*champ->coupe.uu3d[i][n]+weight*champ->coupe.uu3d[i+1][n]);
	 champ->vv[n]=((1.0-weight)*champ->coupe.vv3d[i][n]+weight*champ->coupe.vv3d[i+1][n]);
	 champ->module[n]=sqrt(champ->uu[n]*champ->uu[n]+champ->vv[n]*champ->vv[n]);
	 }
       break;

     default:
       break;
     }
   
   last_index = i;
   
}
