/*
*****************************************************************************
BOUNDARY.CPP
---------------------


*****************************************************************************
*/

#include "lisflood.h"

//-----------------------------------------------------------------------------------
// BOUNDARY CONDITIONS
// Calculate Qx and Qy at edges of the domain in response to boundary
// conditions
void BCs(States *Statesptr,Pars *Parptr,Solver *Solverptr,BoundCs *BCptr,ChannelSegmentType *ChannelSegments,Arrays *Arrptr)
{
	int i, j, BCi, numBCs, p0, p1, sign, dir, edge, pTQ;
	NUMERIC_TYPE h0, h1, z0, z1, hflow, fn, dh, Sf, alpha = C(1e-6), g, q0;
	NUMERIC_TYPE Qlim, *qptr, *qoldptr;

	NUMERIC_TYPE hul, hvl, hvr, hur, hr, hl;
	NUMERIC_TYPE zl, zr;

	//NUMERIC_TYPE shl,shr,aux1,ul,ur,vl,vr,ubarra,vbarra,cbarra,a1,a2,a3,a1m,a2m,a3m,a1l,a1r,a3l,a3r,epsilon,dhu,dhv; CCS threw unreferenced local variable warning so I guess no longer needed?
	//NUMERIC_TYPE alfa1,alfa2,alfa3,e11,e12,e13,e21,e22,e23,e31,e32,e33,f1lp,f2lp,f3lp,f1rp,f2rp,f3rp;	CCS threw unreferenced local variable warning so I guess no longer needed?
	//NUMERIC_TYPE g1lp,g2lp,g3lp,g1rp,g2rp,g3rp; CCS threw unreferenced local variable warning so I guess no longer needed?

	g = Solverptr->g;

	// BCs default to zero flux
	for (j = 0; j < Parptr->ysz; j++) {
		Arrptr->Qx[j*(Parptr->xsz + 1)] = C(0.0);  //zero West boundary
		Arrptr->Qx[Parptr->xsz + j*(Parptr->xsz + 1)] = C(0.0); //zero East boundary
	}
	for (i = 0; i < Parptr->xsz; i++) {
		Arrptr->Qy[i] = C(0.0);  //zero North boundary
		Arrptr->Qy[i + Parptr->ysz*(Parptr->xsz + 1)] = C(0.0); //zero South boundary
	}
	numBCs = 2 * Parptr->xsz + 2 * Parptr->ysz;
	for (BCi = 0; BCi < numBCs; BCi++)
	{
		if (BCptr->BC_Ident[BCi] != NONE0 || Statesptr->Roe == ON) // do nothing if closed boundary and not the Roe solver
		{
			// First for each edge number work out where it is on the boundary,
			// the associated edge pixels and whether it's facing in the x or y
			// direction
			if (BCi <= Parptr->xsz - 1)
			{	// N(j=0) edge
				p0 = BCi;
				p1 = BCi + Parptr->xsz;
				sign = -1;
				qptr = Arrptr->Qy + BCi;
				qoldptr = Arrptr->Qyold + BCi;
				q0 = *qoldptr; // Set old value of Q for acceleration version
				dir = 1;
				pTQ = BCi;
				edge = 1;
			}
			else if (BCi >= Parptr->xsz && BCi <= Parptr->xsz + Parptr->ysz - 1)
			{  // E edge
				p0 = Parptr->xsz - 1 + (BCi - Parptr->xsz)*Parptr->xsz;
				p1 = Parptr->xsz - 2 + (BCi - Parptr->xsz)*Parptr->xsz;
				sign = 1;
				qptr = Arrptr->Qx + Parptr->xsz + (BCi - Parptr->xsz)*(Parptr->xsz + 1);
				qoldptr = Arrptr->Qxold + Parptr->xsz + (BCi - Parptr->xsz)*(Parptr->xsz + 1);
				dir = 2;
				pTQ = Parptr->xsz + (BCi - Parptr->xsz)*(Parptr->xsz + 1);
				q0 = *qoldptr; // Set old value of Q for acceleration version
				edge = 2;
			}
			else if (BCi >= Parptr->xsz + Parptr->ysz && BCi <= 2 * Parptr->xsz + Parptr->ysz - 1)
			{  // S(j=ysz-1) edge
				p0 = 2 * Parptr->xsz + Parptr->ysz - 1 - BCi + (Parptr->ysz - 1)*Parptr->xsz;
				p1 = 2 * Parptr->xsz + Parptr->ysz - 1 - BCi + (Parptr->ysz - 2)*Parptr->xsz;
				sign = 1;
				qptr = Arrptr->Qy + 2 * Parptr->xsz + Parptr->ysz - 1 - BCi + (Parptr->ysz)*(Parptr->xsz + 1);
				qoldptr = Arrptr->Qyold + 2 * Parptr->xsz + Parptr->ysz - 1 - BCi + (Parptr->ysz)*(Parptr->xsz + 1);
				dir = 1;
				pTQ = 2 * Parptr->xsz + Parptr->ysz - 1 - BCi + (Parptr->ysz)*(Parptr->xsz + 1);
				q0 = *qoldptr; // Set old value of Q for acceleration version
				edge = 3;
			}
			else
			{   // W edge
				p0 = 0 + (numBCs - 1 - BCi)*Parptr->xsz;
				p1 = 1 + (numBCs - 1 - BCi)*Parptr->xsz;
				sign = -1;
				qptr = Arrptr->Qx + (numBCs - 1 - BCi)*(Parptr->xsz + 1);
				qoldptr = Arrptr->Qxold + (numBCs - 1 - BCi)*(Parptr->xsz + 1);
				dir = 2;
				pTQ = (numBCs - 1 - BCi)*(Parptr->xsz + 1);
				q0 = *qoldptr; // Set old value of Q for acceleration version
				edge = 4;
			}

			// Now calculate flows
			if (BCptr->BC_Ident[BCi] == FREE1 && Arrptr->H[p0] > Solverptr->DepthThresh)
			{	// FREE boundary
				hflow = Arrptr->H[p0];
				h0 = Arrptr->H[p0];
				h1 = Arrptr->H[p1];
				z0 = Arrptr->DEM[p0];
				z1 = Arrptr->DEM[p1];
				if (Arrptr->Manningsn != NULL) fn = Arrptr->Manningsn[p0]; else fn = Parptr->FPn;

				if (!(Arrptr->ChanMask[p0] != -1))
				{
					if (Statesptr->acceleration == ON || Statesptr->Roe == ON) // Use semi-implicit accceleration formulation
					{
						// if BCptr->BC_Val[BCi] is -1 there is no floodplain slope specifed... use local slope from elevation model (likely to be unstable)
						if (BCptr->BC_Val[BCi] < C(-0.999)) // ie -1 (done like this as NUMERIC_TYPE)
						{
							dh = z0 + h0 - z1 - h1;
							//dh=-h0;  
							Sf = -dh / Parptr->dx;
						}
						else // use floodplain slope
						{
							Sf = BCptr->BC_Val[BCi];
						}
						// multiply flux by -sign and use absolute value of q0 to get flux directions correctly assigned at boundaries
						// fabs on Sf and q0 always results in positive or no flow... sign then sorts out the direction(jcn)
						*qoldptr = sign*(fabs(q0) + fabs(g*Solverptr->Tstep*hflow*Sf)) / (1 + g*Solverptr->Tstep*hflow*fn*fn*fabs(q0) / (pow(hflow, (C(10.) / C(3.)))));
						*qptr = *qoldptr*Parptr->dx; // potntially unnessasary?? check for repetition in UpdateQs (jcn)
					}
					else if (Statesptr->Roe == ON)
					{
						if (edge == 2) *qptr = C(1.75)*sqrt(g)*pow(Arrptr->H[i - 1 + (Parptr->ysz)*(Parptr->xsz)], C(1.5)); // East
						else if (edge == 4)*qptr = C(1.75)*sqrt(g)*pow(Arrptr->H[i + 1 + (Parptr->ysz)*(Parptr->xsz)], C(1.5)); // West
						else if (edge == 3) *qptr = C(1.75)*sqrt(g)*pow(Arrptr->H[i + (Parptr->ysz - 1)*(Parptr->xsz)], C(1.5));	// South			
						else if (edge == 1) *qptr = C(1.75)*sqrt(g)*pow(Arrptr->H[i + (Parptr->ysz + 1)*(Parptr->xsz)], C(1.5));	// North
					}
					else // origional lisflood-fp boundary
					{
						// if BCptr->BC_Val[BCi] is -1 there is no floodplain slope specifed... use local slope from elevation model (origional lisflood)
						if (BCptr->BC_Val[BCi] < C(-0.999)) // ie -1 (done like this as NUMERIC_TYPE)
						{
							// origional lisflood-fp boundary
							dh = -z0 - h0 + z1 + h1;
							Sf = sqrt(fabs(dh) / Parptr->dx);
						}
						else
						{
							Sf = BCptr->BC_Val[BCi]; // sqrt of user specified slope calulated in input.cpp for qlim and adaptive
							dh = BCptr->BC_Val[BCi] * BCptr->BC_Val[BCi] * Parptr->dx; // backcalulate dh for sqrt(user slope)... OK so this is not very efficient could be stored somewhere  
						}

						if (fabs(dh) < Solverptr->dhlin)
						{
							Sf = sqrt(Parptr->dx / Solverptr->dhlin)*(fabs(dh) / Parptr->dx);
							alpha = (pow(hflow, (C(5.) / C(3.)))*Parptr->dx_sqrt) / (fn*sqrt(Solverptr->dhlin));
						}
						else alpha = pow(hflow, (C(5.) / C(3.))) / (C(2.)*fn*Sf);

						if (dh < 0) Sf = -Sf;
						*qptr = sign*pow(hflow, (C(5.) / C(3.)))*Sf*Parptr->dy / fn;
					}
					if (Statesptr->adaptive_ts == ON)
					{
						Solverptr->Tstep = getmin(Solverptr->Tstep, (C(0.25)*Parptr->dy*Parptr->dy / alpha));
						// TJF: added to record Tstep
						if (dir == 1) Arrptr->TRecy[pTQ] = Solverptr->Tstep;
						if (dir == 2) Arrptr->TRecx[pTQ] = Solverptr->Tstep;
					}
					else if (Statesptr->qlim == ON)
					{
						Qlim = Solverptr->Qlimfact*Parptr->dA*fabs(dh) / (8 * Solverptr->Tstep);
						if (fabs(*qptr) > Qlim)
						{
							if (*qptr > 0) *qptr = Qlim;
							if (*qptr < 0) *qptr = -Qlim;
							// TJF: added to record Qlim
							if (dir == 1) Arrptr->LimQy[pTQ] = *qptr;
							if (dir == 2) Arrptr->LimQx[pTQ] = *qptr;
						}
					}
				}
				else *qptr = C(0.0);
				if (dir == 1 && sign == -1 && *qptr > 0) *qptr = C(0.0);
				if (dir == 1 && sign == 1 && *qptr < 0) *qptr = C(0.0);
				if (dir == 2 && sign == -1 && *qptr > 0) *qptr = C(0.0);
				if (dir == 2 && sign == 1 && *qptr<0) *qptr = C(0.0);
			}

			switch (BCptr->BC_Ident[BCi])
			{
				// HFIX boundary
			case HFIX2:
				if (true)//Arrptr->H[p0]>Solverptr->DepthThresh)
				{
					hflow = getmax(Arrptr->H[p0], BCptr->BC_Val[BCi] - Arrptr->DEM[p0]);
					h0 = BCptr->BC_Val[BCi];
					h1 = Arrptr->H[p0];
					z1 = Arrptr->DEM[p0];
					if(hflow<Solverptr->DepthThresh) continue;

//					if (p0 == 7938) {
//					if (p0 == 80) {

//						bool test;
//						test = 1;

						//printf
						//(
						//	//					"hflow: %f\n"
						//	"h1: %f\n"
						//	//					"h1: %f\n"
						//	//					"z1: %f\n"
						//	//					"q_old: %f\n"
						//	//					"d_non_uniform_nghbrs.q: %f\n"
						//	//					"dh: %f\n"
						//	//					"Sf: %f\n"
						//	//					"Manning: %f\n"
						//	"t: %f\n\n"

						//	,
						//	h1, Solverptr->t
						//	//					hflow, h0, h1, z1, q_old, d_non_uniform_nghbrs.q[idx], dh, Sf, Manning, solver.Tstep
						//);


//					}

					if (Arrptr->Manningsn != NULL) fn = Arrptr->Manningsn[p0]; else fn = Parptr->FPn;

					if (!(Arrptr->ChanMask[p0] != -1))
					{
						if (Statesptr->acceleration == ON) // Use semi-implicit accceleration formulation
						{
							dh = h0 - z1 - h1;
							// change slops direction depending on the edge
							if (edge == 1 || edge == 4) Sf = -dh / Parptr->dx;
							else Sf = dh / Parptr->dx;
							// implement momentum equation
							*qoldptr = (q0 - g*Solverptr->Tstep*hflow*Sf) / (C(1.) + g*Solverptr->Tstep*hflow*fn*fn*fabs(q0) / (pow(hflow, (C(10.) / C(3.)))));
							*qptr = *qoldptr*Parptr->dx;
						}
						else // origional lisflood boundary
						{
							dh = -h0 + z1 + h1;
							Sf = sqrt(fabs(dh) / Parptr->dx);

							if (fabs(dh) < Solverptr->dhlin)
							{
								Sf = sqrt(Parptr->dx / Solverptr->dhlin)*(fabs(dh) / Parptr->dx);
								alpha = (pow(hflow, (C(5.) / C(3.)))*Parptr->dx_sqrt) / (fn*sqrt(Solverptr->dhlin));
							}
							else alpha = pow(hflow, (C(5.) / C(3.))) / (C(2.)*fn*Sf);

							if (dh < 0) Sf = -Sf;
							*qptr = sign*pow(hflow, (C(5.) / C(3.)))*Sf*Parptr->dy / fn;
						}
						if (Statesptr->adaptive_ts == ON && hflow > C(0.0))
						{
							Solverptr->Tstep = getmin(Solverptr->Tstep, (C(0.25)*Parptr->dy*Parptr->dy / alpha));
							// TJF: added to record Tstep
							if (dir == 1) Arrptr->TRecy[pTQ] = Solverptr->Tstep;
							if (dir == 2) Arrptr->TRecx[pTQ] = Solverptr->Tstep;
						}
						else if (Statesptr->qlim == ON)
						{
							Qlim = Solverptr->Qlimfact*Parptr->dA*fabs(dh) / (C(8.)*Solverptr->Tstep);
							if (fabs(*qptr) > Qlim)
							{
								//printf("Q limited %" NUM_FMT"->%" NUM_FMT" at t=%i\n",fabs(*qptr),Qlim,ts);
								if (*qptr > 0) *qptr = Qlim;
								if (*qptr < 0) *qptr = -Qlim;
								// TJF: added to record Qlim
								if (dir == 1) Arrptr->LimQy[pTQ] = *qptr;
								if (dir == 2) Arrptr->LimQx[pTQ] = *qptr;
							}
						}
					}
				}
				break;

				// HVAR boundary
			case HVAR3:
				if (true)//Arrptr->H[p0] > Solverptr->DepthThresh)
				{
					//h0=BCVarlist[(int)BC_Val[BCi]][ts];
					h0 = InterpolateTimeSeries(BCptr->BC_TimeSeries[BCi], Solverptr->t);
					hflow = getmax(Arrptr->H[p0], h0 - Arrptr->DEM[p0]);
					h1 = Arrptr->H[p0];
					z1 = Arrptr->DEM[p0];
					if(hflow<Solverptr->DepthThresh) continue;

					if (Arrptr->Manningsn != NULL) fn = Arrptr->Manningsn[p0]; else fn = Parptr->FPn;

					if (!(Arrptr->ChanMask[p0] != -1)){
						if (Statesptr->Roe == ON){

							//north edge
							if (edge == 1){
								z0 = z1 + (Arrptr->DEM[p0] - Arrptr->DEM[p1]);//GUS
								zl = z0;
								zr = z1;
								hl = h0 - z0;
								hr = h1;
								hvr = Arrptr->HV[p0];
								hvl = hvr;
								hur = Arrptr->HU[p0];
								hul = hur;
								//call RoeBCx
								*qptr = RoeBCy(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);
								*qoldptr = *qptr / Parptr->dx;
							}
							//east edge
							if (edge == 2){
								z0 = z1 - (Arrptr->DEM[p1] - Arrptr->DEM[p0]);//GUS
								zl = z1;
								zr = z0;
								hr = h0 - z0;
								hl = h1;
								hvl = Arrptr->HV[p0];
								hvr = hvl;
								hul = Arrptr->HU[p0];
								hur = hul;

								//call RoeBCx
								*qptr = RoeBCx(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);
								*qoldptr = *qptr / Parptr->dx;
							}

							//south edge
							if (edge == 3){
								z0 = z1 + (Arrptr->DEM[p0] - Arrptr->DEM[p1]);//GUS
								zl = z1;
								zr = z0;
								hr = h0 - z0;
								hl = h1;
								hvl = Arrptr->HV[p0];
								hvr = hvl;
								hul = Arrptr->HU[p0];
								hur = hul;
								//call RoeBCx
								*qptr = RoeBCy(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);
								*qoldptr = *qptr / Parptr->dx;
							}

							if (edge == 4){//west edge
								z0 = z1 + (Arrptr->DEM[p0] - Arrptr->DEM[p1]);//GUS
								zl = z0;
								zr = z1;
								hl = h0 - z0;
								hr = h1;
								hvr = Arrptr->HV[p0];
								hvl = hvr;
								hur = Arrptr->HU[p0];
								hul = hur;

								//call RoeBCx
								*qptr = RoeBCx(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);
								*qoldptr = *qptr / Parptr->dx;
							}


						}
						else{
							if (Statesptr->acceleration == ON) // Use semi-implicit accceleration formulation
							{
								*qptr = C(0.0);
								// if(h0>Solverptr->DepthThresh || h1>Solverptr->DepthThresh){
								dh = h0 - z1 - h1;
								// change slops direction depending on the edge
								if (edge == 1 || edge == 4) Sf = -dh / Parptr->dx;
								else Sf = dh / Parptr->dx;
								*qoldptr = (q0 - g*Solverptr->Tstep*hflow*Sf) / (C(1.) + g*Solverptr->Tstep*hflow*fn*fn*fabs(q0) / (pow(hflow, (C(10.) / C(3.)))));
								*qptr = *qoldptr*Parptr->dx;

								// }
							}
							else // origional lisflood boundary
							{
								dh = -h0 + z1 + h1;
								Sf = sqrt(fabs(dh) / Parptr->dx);
								if (fabs(dh) < Solverptr->dhlin)
								{
									Sf = sqrt(Parptr->dx / Solverptr->dhlin)*(fabs(dh) / Parptr->dx);
									alpha = (pow(hflow, (C(5.) / C(3.)))*Parptr->dx_sqrt) / (fn*sqrt(Solverptr->dhlin));
								}
								else alpha = pow(hflow, (C(5.) / C(3.))) / (C(2.)*fn*Sf);

								if (dh < 0) Sf = -Sf;
								*qptr = sign*pow(hflow, (C(5.) / C(3.)))*Sf*Parptr->dy / fn;
							}
							if (Statesptr->adaptive_ts == ON)
							{
								Solverptr->Tstep = getmin(Solverptr->Tstep, (C(0.25)*Parptr->dy*Parptr->dy / alpha));
								// TJF: added to record Tstep
								if (dir == 1) Arrptr->TRecy[pTQ] = Solverptr->Tstep;
								if (dir == 2) Arrptr->TRecx[pTQ] = Solverptr->Tstep;
							}
							else if (Statesptr->qlim == ON)
							{
								Qlim = Solverptr->Qlimfact*Parptr->dA*fabs(dh) / (C(8.)*Solverptr->Tstep);
								if (fabs(*qptr) > Qlim)
								{
									if (*qptr > 0) *qptr = Qlim;
									if (*qptr < 0) *qptr = -Qlim;
									// TJF: added to record Qlim
									if (dir == 1) Arrptr->LimQy[pTQ] = *qptr;
									if (dir == 2) Arrptr->LimQx[pTQ] = *qptr;
								}
							}
						}
					}
				}
				break;


				// QFIX boundary
			case QFIX4:
			{
				*qptr = -BCptr->BC_Val[BCi] * sign*Parptr->dx;
			}
				break;
				// QVAR boundary
			case QVAR5:
			{
				h0 = InterpolateTimeSeries(BCptr->BC_TimeSeries[BCi], Solverptr->t);

				*qptr = -sign*h0*Parptr->dx;
				hflow = getmax(Arrptr->H[p0], h0 - Arrptr->DEM[p0]);

				//GUS CODE
				if (Statesptr->Roe == ON)
				{
					if (edge == 1 && Arrptr->H[p0] > Solverptr->MomentumThresh) // north edge
					{
						h1 = Arrptr->H[p0];
						h0 = h1;
						z1 = Arrptr->DEM[p0];

						z0 = z1 + (Arrptr->DEM[p0] - Arrptr->DEM[p1]);//GUS
						zl = z0;
						zr = 1;
						hl = h0;
						hr = h1;
						hvl = *qptr;
						hvr = Arrptr->HV[p0];
						hul = C(0.0);
						hur = Arrptr->HU[p0];
						//call RoeBCx (without return)
						RoeBCy(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);

					}
					if (edge == 2 && Arrptr->H[p0] > Solverptr->MomentumThresh) // east edge
					{
						h0 = Arrptr->H[p0];
						h1 = h0;
						z0 = Arrptr->DEM[p0];

						z1 = z0 - (Arrptr->DEM[p1] - Arrptr->DEM[p0]);//GUS
						zl = z0;
						zr = z1;
						hl = h0;
						hr = h1;
						hvl = Arrptr->HV[p0];
						hvr = C(0.0);
						hur = *qptr;
						hul = Arrptr->HU[p0];
						//call RoeBCx (without return)
						RoeBCx(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);


					}

					if (edge == 3 && Arrptr->H[p0] > Solverptr->MomentumThresh) // south edge
					{
						h0 = Arrptr->H[p0];
						h1 = h0;
						z0 = Arrptr->DEM[p0];

						z1 = z0 - (Arrptr->DEM[p1] - Arrptr->DEM[p0]);//GUS
						zl = z0;
						zr = z1;
						hl = h0;
						hr = h1;
						hvl = Arrptr->HV[p0];
						hvr = *qptr;
						hur = C(0.0);
						hul = Arrptr->HU[p0];
						//call RoeBCx (without return)
						RoeBCy(edge, p0, p1, pTQ, z0, z1, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);


					}
					if (edge == 4 && Arrptr->H[p0] > Solverptr->MomentumThresh) // west edge
					{
						h1 = Arrptr->H[p0];
						h0 = h1;
						z1 = Arrptr->DEM[p0];

						z0 = z1 + (Arrptr->DEM[p0] - Arrptr->DEM[p1]);//GUS
						zl = z0;
						zr = z1;
						hl = h0;
						hr = h1;
						hvl = C(0.0);
						hvr = Arrptr->HV[p0];
						hul = *qptr;
						hur = Arrptr->HU[p0];
						//call RoeBCx (without return)
						RoeBCx(edge, p0, p1, pTQ, zl, zr, hl, hr, hul, hur, hvl, hvr, Statesptr, Parptr, Solverptr, Arrptr);
					}
				}
			}
				break;
			default:
				break;
			}
			// close boundary in Roe version
			if (Statesptr->Roe == ON)
			{
				if (BCptr->BC_Ident[BCi] == NONE0 && Arrptr->H[p0] > Solverptr->DepthThresh)
				{
					// GHOST boundary optimized solver

					if (edge == 1) // north edge
					{
						hr = Arrptr->H[p0];
						hvr = Arrptr->HV[p0];

						Arrptr->FHUy[pTQ] = C(0.);
						Arrptr->FHVy[pTQ] = hvr*(hvr / hr - pow(g*hr, C(0.5))) + C(0.5)*g*hr*hr;

					}
					else if (edge == 2) // east edge
					{
						hl = Arrptr->H[p0];
						hul = Arrptr->HU[p0];

						Arrptr->FHUx[pTQ] = hul*(hul / hl + pow(g*hl, C(0.5))) + C(0.5)*g*hl*hl;
						Arrptr->FHVx[pTQ] = C(0.);
					}
					else if (edge == 3) // south edge
					{
						hl = Arrptr->H[p0];
						hvl = Arrptr->HV[p0];

						Arrptr->FHUy[pTQ] = C(0.);
						Arrptr->FHVy[pTQ] = hvl*(hvl / hl + pow(g*hl, C(0.5))) + C(0.5)*g*hl*hl;
					}
					else // west edge
					{
						hr = Arrptr->H[p0];
						hur = Arrptr->HU[p0];

						Arrptr->FHUx[pTQ] = hur*(hur / hr - pow(g*hr, C(0.5))) + C(0.5)*g*hr*hr;
						Arrptr->FHVx[pTQ] = C(0.);
					}
				}
			}
		}
	}
	return;
}

//----------------------------------------------------------------------------

NUMERIC_TYPE RoeBCx(int edge, int p0, int p1, int pq0, NUMERIC_TYPE zl, NUMERIC_TYPE zr, NUMERIC_TYPE hl, NUMERIC_TYPE hr, NUMERIC_TYPE hul, NUMERIC_TYPE hur, NUMERIC_TYPE hvl, NUMERIC_TYPE hvr,States *Statesptr,Pars *Parptr,Solver *Solverptr,Arrays *Arrptr)
{
  NUMERIC_TYPE hflow,dh=C(0.0),Q,g,dt;

  NUMERIC_TYPE dl,s0;
  NUMERIC_TYPE dtol;
  NUMERIC_TYPE shl,shr,aux1,ul,ur,vl,vr,ubarra,vbarra,cbarra,a1,a2,a3,a1m,a2m,a3m,a1l,a1r,a3l,a3r,epsilon,dhu,dhv;
  NUMERIC_TYPE alfa1,alfa2,alfa3,e11,e12,e13,e21,e22,e23,e31,e32,e33,f1lp,f2lp,f3lp,f1rp,f2rp,f3rp,fctm,wsurface;//beta1,beta2,beta3; CCS threw unreferenced local variable warning so I guess no longer needed?
  NUMERIC_TYPE maximum(NUMERIC_TYPE a,NUMERIC_TYPE b,NUMERIC_TYPE c);

    // Predefined-variables
    g=Solverptr->g;
    dt=Solverptr->Tstep;
    dl=Parptr->dx ;
    dtol=Solverptr->DepthThresh; // same as LF-FP


    // Type of flow cases:
    //----------------------------------------------------------------------------------------

	if (hl >= dtol && hr >= dtol){	/* both normal */

		       	shr= pow(hr,C(0.5));
				shl= pow(hl,C(0.5));
				aux1= shr + shl ;

				ul= hul/hl;
				ur= hur/hr;
				vl= hvl/hl;
				vr= hvr/hr;

				ubarra= (shr*ur+shl*ul)/aux1;
				vbarra= (shr*vr+shl*vl)/aux1;
				cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));

				a1=ubarra+cbarra ;
				a2=ubarra ;
				a3=ubarra-cbarra ;

				a1m=fabs(a1) ;
				a2m=fabs(a2) ;
				a3m=fabs(a3) ;

				a1l=ul+pow(g*hl,C(0.5)) ;
				a1r=ur+pow(g*hr,C(0.5)) ;

				epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ;
				a1m=(a1m >= epsilon) ? a1m : epsilon  ;

				a3l=ul-pow(g*hl,C(0.5)) ;
				a3r=ur-pow(g*hr,C(0.5)) ;

				epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ;
				a3m=(a3m >= epsilon) ? a3m : epsilon  ;

				dh=hr-hl ;
				dhu=hur-hul ;
				dhv=hvr-hvl ;

				alfa1=C(0.5)*dh+C(0.5)*(dhu-ubarra*dh)/cbarra ;
				alfa2=(dhv-vbarra*dh)/cbarra ;
				alfa3=C(0.5)*dh-C(0.5)*(dhu-ubarra*dh)/cbarra ;

				e11=C(1.) ;
				e12=ubarra+cbarra;
				e13=vbarra ;

				e21=C(0.) ;
				e22=C(0.) ;
				e23=cbarra;

				e31=C(1.) ;
				e32=ubarra-cbarra;
				e33=vbarra;

				/* flux variables */

				f1lp=hul ;
				f2lp=hul*ul+C(0.5)*g*hl*hl ;
				f3lp=hul*vl ;

				f1rp=hur ;
				f2rp=hur*ur+C(0.5)*g*hr*hr ;
				f3rp=hur*vr ;


				Arrptr->FHx[pq0]=C(0.5)*(f1rp+f1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
				Arrptr->FHUx[pq0]=C(0.5)*(f2rp+f2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
				Arrptr->FHVx[pq0]=C(0.5)*(f3rp+f3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;

				// East or West
				if(edge==2){//east
					s0=(Arrptr->DEM[p1]-Arrptr->DEM[p0])/dl;
					Arrptr->RSHU[p0]=C(0.5)*g*hl*s0 ;
					Arrptr->RSHV[p0]=C(0.0);
				}
				else if(edge==4){//west
					s0=-(Arrptr->DEM[p1]-Arrptr->DEM[p0])/dl;
					Arrptr->LSHU[p0]=C(0.5)*g*hr*s0 ;
					Arrptr->LSHV[p0]=C(0.0);
				}



			}
			else if(hl < dtol && hr > dtol){  /* left dry  */

				fctm=C(1.)-pow(C(1.),C(0.5))*fabs(hur)/(pow(g,C(0.5))*pow(hr,C(1.5))) ;

				if (fctm < C(0.)) fctm=C(0.);

				//if (hr*fctm+Arrptr->DEM[p1] > Arrptr->DEM[p0]  && (hur/hr-pow(g*hr,C(0.5))) <= C(0.) ){	/* overtopping */
				if (hr*fctm+zr > zl  && (hur/hr-pow(g*hr,C(0.5))) <= C(0.) ){	/* overtopping */

					wsurface=(zl-hr-zr)/dl ;

					Arrptr->FHx[pq0]=hur ;
					Arrptr->FHUx[pq0]=hur*hur/hr ;
					Arrptr->FHVx[pq0]=hvr*hur/hr ;

					Arrptr->H[p0]=dtol;

					// west or east?
					if(edge==2){//east
						Arrptr->RSHU[p0]=g*hr*wsurface ;
						Arrptr->RSHV[p0]=C(0.0);
					}
					else if(edge==4){//west
						Arrptr->LSHU[p0]=g*hr*wsurface ;
						Arrptr->LSHV[p0]=C(0.0);
					}


				}
				else{					/* wall */
						// To verify that left cell is not updated
						// WDC modifies Arrptr->FHx[pq0]=0 ;
/*
						if (hur <= C(0.)){
							Arrptr->FHUx[pq0]=hur*hur/hr ;  // remind (-) sign while upgrade
							Arrptr->FHVx[pq0]=C(0.) ;
						}
						else
						{
							Arrptr->FHUx[pq0]=C(0.) ;
							Arrptr->FHVx[pq0]=C(0.) ;
						}

						//Raw formulation from Brufau (Dont add RSH[p0] for avoiding UpdateRoeH)


							ubarra= ur;
							cbarra= pow(C(0.5)*g*hr,C(0.5));

							if (z0 < z1 + hr*(C(1.)+fabs(ubarra)/cbarra) && fabs(ubarra) < cbarra){
								Arrptr->FHx[pq0]=C(0.5)*(cbarra*(z0-z1)-hr*(fabs(ubarra)+cbarra));
							}
							else{
								Arrptr->FHx[pq0]=0 ;
							}
							// If z1 >> z0  -> great losses
							// If ubarra=0  what happens
*/

					if (Statesptr->Roe_slow==ON)
					{
							hl = hr;
							hul = -hur;
							hvl = hvr;

							//s0=-(Arrptr->DEM[p1+1]-Arrptr->DEM[p1])/dl;
							//Arrptr->RSHU[p0]=C(0.0) ;
							//if (s0 >= 0) Arrptr->LSHU[p1]=C(0.0);
							//else if (Arrptr->H[p1+1] < dtol) Arrptr->LSHU[p1]=C(0.0);
							//else Arrptr->LSHU[p1]=C(0.5)*g*hr*s0;

							Arrptr->RSHV[p0]=C(0.0) ;
							//Arrptr->LSHV[p1]=C(0.0) ;

							//Arrptr->LSHU[p1]=C(0.0) ;
							//Arrptr->LSHV[p1]=C(0.0) ;
							Arrptr->RSHU[p0]=C(0.0) ;//GUSTAVO
							//Arrptr->LSHU[p1]=C(0.0) ;//GUSTAVO

							shr= pow(hr,C(0.5)); shl= pow(hl,C(0.5)); aux1= shr + shl ;
							ul= hul/hl; ur= hur/hr; vl= hvl/hl; vr= hvr/hr;
							ubarra= (shr*ur+shl*ul)/aux1; vbarra= (shr*vr+shl*vl)/aux1; cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));
							a1=ubarra+cbarra ; a2=ubarra ; a3=ubarra-cbarra ;
							a1m=fabs(a1) ; a2m=fabs(a2) ; a3m=fabs(a3) ;
							a1l=ul+pow(g*hl,C(0.5)) ; a1r=ur+pow(g*hr,C(0.5)) ;
							epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ;
							a1m=(a1m >= epsilon) ? a1m : epsilon  ;
							a3l=ul-pow(g*hl,C(0.5)) ; a3r=ur-pow(g*hr,C(0.5)) ;
							epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ; a3m=(a3m >= epsilon) ? a3m : epsilon  ;
							dh=hr-hl ; dhu=hur-hul ; dhv=hvr-hvl ;
							alfa1=C(0.5)*dh+C(0.5)*(dhu-ubarra*dh)/cbarra ;
							alfa2=(dhv-vbarra*dh)/cbarra ;
							alfa3=C(0.5)*dh-C(0.5)*(dhu-ubarra*dh)/cbarra ;
							e11=C(1.) ; e12=ubarra+cbarra; e13=vbarra ;
							e21=C(0.) ; e22=C(0.) ; e23=cbarra;
							e31=C(1.) ; e32=ubarra-cbarra; e33=vbarra;
							/* flux variables */
							f1lp=hul ; f2lp=hul*ul+C(0.5)*g*hl*hl ; f3lp=hul*vl ;
							f1rp=hur ; f2rp=hur*ur+C(0.5)*g*hr*hr ; f3rp=hur*vr ;
							Arrptr->FHx[pq0]=C(0.5)*(f1rp+f1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
							Arrptr->FHUx[pq0]=C(0.5)*(f2rp+f2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
							Arrptr->FHVx[pq0]=C(0.5)*(f3rp+f3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;
					}
					else
					{
						                        // FLAT SOLVER

						//if (hur <= C(0.)){
							Arrptr->FHUx[pq0]=hur*(hur/hr-pow(g*hr,C(0.5)))+C(0.5)*g*hr*hr ;  // remind (-) sign while upgrade
							Arrptr->FHVx[pq0]=C(0.) ;
							Arrptr->FHx[pq0]=C(0.);//GUSTAVO

							Arrptr->RSHV[p0]=C(0.0) ;//GUSTAVO
							//Arrptr->LSHV[p1]=C(0.0) ;//GUSTAVO
							Arrptr->RSHU[p0]=C(0.0) ;//GUSTAVO
							//Arrptr->LSHU[p1]=C(0.0) ;//GUSTAVO
						//}
						//else
						//{
						//	Arrptr->FHUx[pq0]=C(0.) ;
						//	Arrptr->FHVx[pq0]=C(0.) ;
						//}
					}
				}
			}
			else if(hr < dtol && hl > dtol){	/* right dry */

				fctm=C(1.)-pow(C(1.),C(0.5))*fabs(hul)/(pow(g,C(0.5))*pow(hl,C(1.5))) ;

				if (fctm < C(0.)) fctm=C(0.);

				if (hl*fctm+zl  > zr  && (hul/hl+pow(g*hl,C(0.5))) >= C(0.)){	/* overtopping */

					wsurface=(hl+zl-zr)/dl;

					Arrptr->FHx[pq0]=hul ;
					Arrptr->FHUx[pq0]=hul*hul/hl ;
					Arrptr->FHVx[pq0]=hvl*hul/hl ;

					Arrptr->H[p0]=dtol;

					// east or west
					if(edge==2){//east
						Arrptr->RSHU[p0]=g*hl*wsurface ;
						Arrptr->RSHV[p0]=C(0.0) ;
					}
					else if (edge==4){
						Arrptr->LSHU[p0]=g*hr*wsurface ;
						Arrptr->LSHV[p0]=C(0.0);
					}



				}
				else
				{   				/* wall  */

					/* wall at j+1 */

						// WDC modifies Arrptr->FHx[pq0]=0 ;
					if (Statesptr->Roe_slow==ON)
					{
					hr = hl;
					hur = -hul;
					hvr = hvl;

					//Arrptr->RSHU[p0]=C(0.0) ;
					//if (Arrptr->LSHU[p0] > 0) Arrptr->RSHU[p0];
					//else Arrptr->RSHU[p0]= -Arrptr->LSHU[p0] ;

					//s0=-(Arrptr->DEM[p1+1]-Arrptr->DEM[p1])/dl;
					//if (Arrptr->LSHU[p0] <= 0) Arrptr->RSHU[p0] = C(0.0);
					//else if (Arrptr->H[p0-1] < dtol) Arrptr->RSHU[p0] = C(0.0); // if other side is also dry assume zero
					//else Arrptr->RSHU[p0]=Arrptr->LSHU[p0] ;
					//Arrptr->LSHU[p1]=C(0.0) ; //C(0.5)*g*hr*s0;

					Arrptr->RSHV[p0]=C(0.0) ;
					//Arrptr->LSHV[p1]=C(0.0) ;

					//Arrptr->RSHU[p0]=C(0.0) ;
					//Arrptr->RSHV[p0]=C(0.0) ;
					Arrptr->RSHU[p0]=C(0.0) ;//GUSTAVO
					//Arrptr->LSHU[p1]=C(0.0) ;//GUSTAVO

					shr= pow(hr,C(0.5)); shl= pow(hl,C(0.5)); aux1= shr + shl ;
					ul= hul/hl; ur= hur/hr; vl= hvl/hl; vr= hvr/hr;
					ubarra= (shr*ur+shl*ul)/aux1; vbarra= (shr*vr+shl*vl)/aux1; cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));
					a1=ubarra+cbarra ; a2=ubarra ; a3=ubarra-cbarra ;
					a1m=fabs(a1) ; a2m=fabs(a2) ; a3m=fabs(a3) ;
					a1l=ul+pow(g*hl,C(0.5)) ; a1r=ur+pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ;
					a1m=(a1m >= epsilon) ? a1m : epsilon  ;
					a3l=ul-pow(g*hl,C(0.5)) ; a3r=ur-pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ; a3m=(a3m >= epsilon) ? a3m : epsilon  ;
					dh=hr-hl ; dhu=hur-hul ; dhv=hvr-hvl ;
					alfa1=C(0.5)*dh+C(0.5)*(dhu-ubarra*dh)/cbarra ;
					alfa2=(dhv-vbarra*dh)/cbarra ;
					alfa3=C(0.5)*dh-C(0.5)*(dhu-ubarra*dh)/cbarra ;
					e11=C(1.) ; e12=ubarra+cbarra; e13=vbarra ;
					e21=C(0.) ; e22=C(0.) ; e23=cbarra;
					e31=C(1.) ; e32=ubarra-cbarra; e33=vbarra;
					/* flux variables */
					f1lp=hul ; f2lp=hul*ul+C(0.5)*g*hl*hl ; f3lp=hul*vl ;
					f1rp=hur ; f2rp=hur*ur+C(0.5)*g*hr*hr ; f3rp=hur*vr ;
					Arrptr->FHx[pq0]=C(0.5)*(f1rp+f1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
					Arrptr->FHUx[pq0]=C(0.5)*(f2rp+f2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
					Arrptr->FHVx[pq0]=C(0.5)*(f3rp+f3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;
					}
					else
					{
						/* wall at j+1 */

						// WDC modifies Arrptr->FHx[pq0]=0 ;

						//if (hul >= C(0.)){

							Arrptr->FHUx[pq0]=hul*(hul/hl+pow(g*hl,C(0.5)))+C(0.5)*g*hl*hl;
							Arrptr->FHVx[pq0]=C(0.);

							Arrptr->FHx[pq0]=C(0.);//GUSTAVO

							Arrptr->RSHV[p0]=C(0.0) ;//GUSTAVO
							//Arrptr->LSHV[p1]=C(0.0) ;//GUSTAVO
							Arrptr->RSHU[p0]=C(0.0) ;//GUSTAVO
							//Arrptr->LSHU[p1]=C(0.0) ;//GUSTAVO
						//}
						//else{

						//	Arrptr->FHUx[pq0]=C(0.);
						//	Arrptr->FHVx[pq0]=C(0.);

						//}
					}
					/*
						if (hul >= C(0.)){

							Arrptr->FHUx[pq0]=hul*hul/hl;
							Arrptr->FHVx[pq0]=C(0.);
						}
						else{

							Arrptr->FHUx[pq0]=C(0.);
							Arrptr->FHVx[pq0]=C(0.);

						}

						// To verify that right cell is not updated

						Arrptr->RSHU[p0]=0 ;
						Arrptr->RSHV[p0]=0 ;


						// WDC from Brufau et Al


							//Raw formulation from Brufau (Dont add RSH[p0] for avoiding UpdateRoeH)


							ubarra= ul;
							cbarra= pow(C(0.5)*g*hl,C(0.5));

							if (z1 < z0 + hl*(C(1.)+fabs(ubarra)/cbarra) && fabs(ubarra) < cbarra){

								Arrptr->FHx[pq0]=C(-0.5)*(cbarra*(z1-z0)-hl*(fabs(ubarra)+cbarra));
							}
							else{
								Arrptr->FHx[pq0]=C(0.);
							}

							// If z1 >> z0  -> great losses
							// If ubarra=0  what happens


						// 2nd Add all betas to Arrptr->RSHU[p0], Arrptr->RSHV[p0];


							beta1=C(0.);
							beta2=C(0.);
							beta3=C(0.);	*/
				}
			}
    //----------------------------------------------------------------------------------------
  Q=Arrptr->FHx[pq0]*dl ; // origional version included addition *dl but removed for compatability with UpdateH JCN

  // option to save V's
  if (Statesptr->voutput==ON)
  {
	  if (Q!=C(0.0))
	  {
	    hflow=getmax(zl+hl,zr+hr)-getmax(zl,zr);
        hflow=getmax(hflow,0);
        hflow=getmin(hflow,Solverptr->MaxHflow);
	    // calc Vx
	    Arrptr->Vx[pq0]=Arrptr->FHx[pq0]/hflow;
	    // get max Vx
	    Arrptr->maxVx[pq0]=getmax(Arrptr->maxVx[pq0],fabs(Arrptr->Vx[pq0]));
	  }
	  else Arrptr->Vx[pq0]=C(0.0);
  }
  return(Q);
}
//-----------------------------------------------------------------------------

NUMERIC_TYPE RoeBCy(int edge, int p0, int p1, int pq0, NUMERIC_TYPE zl, NUMERIC_TYPE zr, NUMERIC_TYPE hl, NUMERIC_TYPE hr, NUMERIC_TYPE hul, NUMERIC_TYPE hur, NUMERIC_TYPE hvl, NUMERIC_TYPE hvr,States *Statesptr,Pars *Parptr,Solver *Solverptr,Arrays *Arrptr)
{
  NUMERIC_TYPE hflow,dh=C(0.0),Q=C(0.0),g,dt;


  NUMERIC_TYPE dl,s0;
  NUMERIC_TYPE dtol;
  NUMERIC_TYPE shl,shr,aux1,ul,ur,vl,vr,ubarra,vbarra,cbarra,a1,a2,a3,a1m,a2m,a3m,a1l,a1r,a3l,a3r,epsilon,dhu,dhv;
  NUMERIC_TYPE alfa1,alfa2,alfa3,e11,e12,e13,e21,e22,e23,e31,e32,e33,g1lp,g2lp,g3lp,g1rp,g2rp,g3rp,fctm,wsurface;
  NUMERIC_TYPE maximum(NUMERIC_TYPE a,NUMERIC_TYPE b,NUMERIC_TYPE c);

  // Indexing
  //p0=i+j*Parptr->xsz;
  //p1=i+(j+1)*Parptr->xsz;
  //pq0=i+(j+1)*(Parptr->xsz+1);

  // Predefined-variables

  g=Solverptr->g;
  dt=Solverptr->Tstep;
  dl=Parptr->dx ;

  dtol=Solverptr->DepthThresh ; // same as LF-FP ?


    // Type of flow cases:

    //----------------------------------------------------------------------------------------

    		if (hl >= dtol && hr >= dtol){	/* both normal */

				shr= pow(hr,C(0.5));
				shl= pow(hl,C(0.5));
				aux1= shr + shl ;

				ul= hul/hl;
				ur= hur/hr;
				vl= hvl/hl;
				vr= hvr/hr;

				ubarra= (shr*ur+shl*ul)/aux1;
				vbarra= (shr*vr+shl*vl)/aux1;
				cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));

				a1=vbarra+cbarra ;
				a2=vbarra ;
				a3=vbarra-cbarra ;

				a1m=fabs(a1) ;
				a2m=fabs(a2) ;
				a3m=fabs(a3) ;

				a1l=vl+pow(g*hl,C(0.5)) ;
				a1r=vr+pow(g*hr,C(0.5)) ;

				epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ;
				a1m=(a1m >= epsilon) ? a1m : epsilon  ;

				a3l=vl-pow(g*hl,C(0.5)) ;
				a3r=vr-pow(g*hr,C(0.5)) ;

				epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ;
				a3m=(a3m >= epsilon) ? a3m : epsilon  ;

				dh=hr-hl ;
				dhu=hur-hul ;
				dhv=hvr-hvl ;

				alfa1=C(0.5)*dh+C(0.5)*(dhv-vbarra*dh)/cbarra ;
				alfa2=-(dhu-ubarra*dh)/cbarra ;
				alfa3=C(0.5)*dh-C(0.5)*(dhv-vbarra*dh)/cbarra ;

				e11=C(1.) ;
				e12=ubarra;
				e13=vbarra+cbarra ;

				e21=C(0.) ;
				e22=-cbarra ;
				e23=C(0.);

				e31=C(1.) ;
				e32=ubarra ;
				e33=vbarra-cbarra ;

				/* flux variables */
				g1lp=hvl ;
				g2lp=hul*vl ;
				g3lp=hvl*vl+C(0.5)*g*hl*hl ;

				g1rp=hvr ;
				g2rp=hur*vr ;
				g3rp=hvr*vr+C(0.5)*g*hr*hr ;

				Arrptr->FHy[pq0]=C(0.5)*(g1rp+g1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
				Arrptr->FHUy[pq0]=C(0.5)*(g2rp+g2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
				Arrptr->FHVy[pq0]=C(0.5)*(g3rp+g3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;

				//printf("**** FHVy %" NUM_FMT" ****\n",Arrptr->FHVy[pq0]);

				//s0=-(z1-z0)/dl;//GUS: s0 will be always zero, but I kept this just in case in the future someone is interested in providing a different bed elevation in the ghost cell
				//Arrptr->BSHU[p0]=C(0.) ;
				//Arrptr->TSHU[p1]=C(0.) ;
				//Arrptr->BSHV[p0]=C(0.5)*g*hl*s0 ;
				//Arrptr->TSHV[p1]=C(0.5)*g*hr*s0 ;

				// IF north or south?
				if(edge==1){//EDGE==North
					s0=-(Arrptr->DEM[p1]-Arrptr->DEM[p0])/Parptr->dx;//extrapolating slope
					Arrptr->TSHV[p0]=C(0.5)*g*hr*s0 ;// GUS temporary
				}
				else if(edge==3){//EDGE==south
					s0=(Arrptr->DEM[p1]-Arrptr->DEM[p0])/Parptr->dx;//extrapolating slope
					Arrptr->BSHV[p0]=C(0.5)*g*hl*s0 ;// GUS temporary
				}
			}
			else if(hl < dtol && hr > dtol){		        /* left dry  */

				fctm=C(1.)-pow(C(1.),C(0.5))*fabs(hvr)/(pow(g,C(0.5))*pow(hr,C(1.5))) ;

				if (fctm < C(0.)) fctm=C(0.);

				if (hr*fctm+zr > zl && (hvr/hr-pow(g*hr,C(0.5)))<= C(0.)){	/* overtopping */

					wsurface=(hl+zl-hr-zr)/dl;

					Arrptr->FHy[pq0]=hvr ;
					Arrptr->FHUy[pq0]=hvr*hur/hr ;
					Arrptr->FHVy[pq0]=hvr*hvr/hr ;

					Arrptr->H[p0]=dtol;

					if(edge==1){//north edge
						Arrptr->TSHV[p0]=g*hr*wsurface ;
						Arrptr->TSHU[p0]=C(0.) ;
					}
					else if(edge==3){//south edge
						Arrptr->BSHV[p0]=g*hr*wsurface ;
						Arrptr->BSHU[p0]=C(0.) ;
					}


				}
				else
				{					/* wall */

						// To verify that left cell is not updated

						//Arrptr->FHy[pq0]=0 ;
					if (Statesptr->Roe_slow==ON)
					{
					hl = hr;
					hul = hur;
					hvl = -hvr;

					//s0=-(Arrptr->DEM[p1]-Arrptr->DEM[p1+Parptr->xsz])/dl; //work out slope on other side
					//if (s0 < 0) Arrptr->TSHV[p1]=C(0.0); // if away from boundary use zero
					//else Arrptr->TSHV[p1]=C(-0.5)*g*hr*s0 ; // if towrads boundary balance woth ghost cell
					//s0=-(Arrptr->DEM[p1+Parptr->xsz]-Arrptr->DEM[p1])/dl;

					Arrptr->BSHU[p0]=C(0.0) ;
					//Arrptr->TSHU[p1]=C(0.0) ;

					Arrptr->BSHV[p0]=C(0.0) ;
					//if (s0 >= 0) Arrptr->TSHV[p1]=C(0.0);
					//else if (Arrptr->H[p1+Parptr->xsz] < dtol) Arrptr->TSHV[p1]=C(0.0);
					//else Arrptr->TSHV[p1]=C(0.5)*g*hr*s0 ;

					//Arrptr->TSHU[p1]=C(0.0) ;
					//Arrptr->TSHV[p1]=C(0.0) ;

					// implement solver for dry edge
					shr= pow(hr,C(0.5)); shl= pow(hl,C(0.5)); aux1= shr + shl ;
					ul= hul/hl; ur= hur/hr; vl= hvl/hl; vr= hvr/hr;
					ubarra= (shr*ur+shl*ul)/aux1; vbarra= (shr*vr+shl*vl)/aux1;	cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));
					a1=vbarra+cbarra ; a2=vbarra ; a3=vbarra-cbarra ;
					a1m=fabs(a1) ; a2m=fabs(a2) ; a3m=fabs(a3) ;
					a1l=vl+pow(g*hl,C(0.5)) ; a1r=vr+pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ; a1m=(a1m >= epsilon) ? a1m : epsilon  ;
					a3l=vl-pow(g*hl,C(0.5)) ; a3r=vr-pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ;	a3m=(a3m >= epsilon) ? a3m : epsilon  ;
					dh=hr-hl ; dhu=hur-hul ; dhv=hvr-hvl ;

					alfa1=C(0.5)*dh+C(0.5)*(dhv-vbarra*dh)/cbarra ; alfa2=-(dhu-ubarra*dh)/cbarra ; alfa3=C(0.5)*dh-C(0.5)*(dhv-vbarra*dh)/cbarra ;
					e11=C(1.) ; e12=ubarra; e13=vbarra+cbarra ;
					e21=C(0.) ; e22=-cbarra ; e23=C(0.);
					e31=C(1.) ; e32=ubarra ; e33=vbarra-cbarra ;
					/* flux variables */
					g1lp=hvl ; g2lp=hul*vl ; g3lp=hvl*vl+C(0.5)*g*hl*hl ;
					g1rp=hvr ; g2rp=hur*vr ; g3rp=hvr*vr+C(0.5)*g*hr*hr ;
					Arrptr->FHy[pq0]=C(0.5)*(g1rp+g1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
					Arrptr->FHUy[pq0]=C(0.5)*(g2rp+g2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
					Arrptr->FHVy[pq0]=C(0.5)*(g3rp+g3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;
					}
					else
					{
						// To verify that left cell is not updated

						//Arrptr->FHy[pq0]=0 ;

						//if (hvr <= 0){

							Arrptr->FHUy[pq0]=C(0.);
							Arrptr->FHVy[pq0]=hvr*(hvr/hr-pow(g*hr,C(0.5)))+C(0.5)*g*hr*hr; // remind (-) sign while upgrade
							Arrptr->FHy[pq0]=C(0.);//GUS

							Arrptr->BSHU[p0]=C(0.0) ;//GUS
							Arrptr->BSHV[p0]=C(0.0) ;//GUS
							//Arrptr->TSHU[p1]=C(0.0) ;//GUS
							//Arrptr->TSHV[p1]=C(0.0) ;//GUS

					}
				}
			}
			else if(hr < dtol && hl > dtol){			/* right dry */


				fctm=C(1.)-pow(C(1.),C(0.5))*fabs(hvl)/(pow(g,C(0.5))*pow(hl,C(1.5))) ;

				if (fctm < C(0.)) fctm=C(0.);

				if (hl*fctm+zl > zr && (hvl/hl +pow(g*hl,C(0.5)) ) >= C(0.)){	/* overtopping */


					wsurface=(hl+zl-hr-zr)/dl;

					if(fabs(hvl/hl)<C(0.0000001)){// if velocity "equals" zero, use LISFLOOD inertial formulation (with q0=0)
						hflow=getmax(zl+hl,zr+hr)-getmax(zl,zr);
						hflow=getmax(hflow,0);
						Arrptr->FHy[pq0]=(g*dt*hflow*wsurface);
						Arrptr->FHUy[pq0]=Arrptr->FHy[pq0]*hul/hl ;
						Arrptr->FHVy[pq0]=Arrptr->FHy[pq0]*Arrptr->FHy[pq0]/hl ;
						Arrptr->H[p0]=dtol;// why???

					}
					else{
						Arrptr->FHy[pq0]=hvl ;
						Arrptr->FHUy[pq0]=hvl*hul/hl ;
						Arrptr->FHVy[pq0]=hvl*hvl/hl ;

						Arrptr->H[p0]=dtol;
					}

					if(edge==1){
						Arrptr->TSHV[p0]=g*hl*wsurface ;
						Arrptr->TSHU[p0]=C(0.0) ;
					}
					else if (edge==3){
						Arrptr->BSHV[p0]=g*hl*wsurface ;
						Arrptr->BSHU[p0]=g*hl*wsurface ;
					}



				}
				else
				{   				/* wall  */

					/* wall at j+1 */

						// To verify that right cell is not updated
					if (Statesptr->Roe_slow==ON)
					{
					hr = hl;
					hur = hul;
					hvr = -hvl;

					//Arrptr->BSHU[p0]=C(0.0);
					//if (Arrptr->TSHV[p0] > 0) Arrptr->BSHV[p0] = C(0.0);
					//else Arrptr->BSHV[p0]= -Arrptr->TSHV[p0];

					//s0=-(Arrptr->DEM[p1+Arrptr->xsz]-Arrptr->DEM[p1])/dl;

					//Arrptr->BSHU[p0]=C(0.0) ;
					//Arrptr->TSHU[p1]=C(0.0) ;

					//if (Arrptr->TSHV[p0] <= 0) Arrptr->BSHV[p0]=C(0.0);
					//else Arrptr->BSHV[p0]=Arrptr->TSHV[p0] ;
					//Arrptr->TSHV[p1]=C(0.0);

					Arrptr->BSHU[p0]=C(0.0);
					Arrptr->BSHV[p0]=C(0.0);

					// implement solver for dry edge
					shr= pow(hr,C(0.5)); shl= pow(hl,C(0.5)); aux1= shr + shl ;
					ul= hul/hl; ur= hur/hr; vl= hvl/hl; vr= hvr/hr;
					ubarra= (shr*ur+shl*ul)/aux1; vbarra= (shr*vr+shl*vl)/aux1;	cbarra= pow(C(0.5)*g*(hl+hr),C(0.5));
					a1=vbarra+cbarra ; a2=vbarra ; a3=vbarra-cbarra ;
					a1m=fabs(a1) ; a2m=fabs(a2) ; a3m=fabs(a3) ;
					a1l=vl+pow(g*hl,C(0.5)) ; a1r=vr+pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a1-a1l),(a1r-a1)) ; a1m=(a1m >= epsilon) ? a1m : epsilon  ;
					a3l=vl-pow(g*hl,C(0.5)) ; a3r=vr-pow(g*hr,C(0.5)) ;
					epsilon=maximum(C(0.),(a3-a3l),(a3r-a3)) ;	a3m=(a3m >= epsilon) ? a3m : epsilon  ;
					dh=hr-hl ; dhu=hur-hul ; dhv=hvr-hvl ;

					alfa1=C(0.5)*dh+C(0.5)*(dhv-vbarra*dh)/cbarra ; alfa2=-(dhu-ubarra*dh)/cbarra ; alfa3=C(0.5)*dh-C(0.5)*(dhv-vbarra*dh)/cbarra ;
					e11=C(1.) ; e12=ubarra; e13=vbarra+cbarra ;
					e21=C(0.) ; e22=-cbarra ; e23=C(0.);
					e31=C(1.) ; e32=ubarra ; e33=vbarra-cbarra ;
					/* flux variables */
					g1lp=hvl ; g2lp=hul*vl ; g3lp=hvl*vl+C(0.5)*g*hl*hl ;
					g1rp=hvr ; g2rp=hur*vr ; g3rp=hvr*vr+C(0.5)*g*hr*hr ;
					Arrptr->FHy[pq0]=C(0.5)*(g1rp+g1lp-a1m*alfa1*e11-a2m*alfa2*e21-a3m*alfa3*e31) ;
					Arrptr->FHUy[pq0]=C(0.5)*(g2rp+g2lp-a1m*alfa1*e12-a2m*alfa2*e22-a3m*alfa3*e32) ;
					Arrptr->FHVy[pq0]=C(0.5)*(g3rp+g3lp-a1m*alfa1*e13-a2m*alfa2*e23-a3m*alfa3*e33) ;
					}
					else
					{
						/* wall at j+1 */

						// To verify that right cell is not updated

						//Arrptr->FHy[pq0]=0 ;

						//if (hvl >= 0){

							Arrptr->FHUy[pq0]=C(0.);
							Arrptr->FHVy[pq0]=hvl*(hvl/hl+pow(g*hl,C(0.5)))+C(0.5)*g*hl*hl;
							Arrptr->FHy[pq0]=C(0.);//GUS

							//Arrptr->TSHU[p1]=C(0.0) ;//GUS
							//Arrptr->TSHV[p1]=C(0.0);//GUS
							Arrptr->BSHU[p0]=C(0.0);//GUS
							Arrptr->BSHV[p0]=C(0.0);//GUS


					}

				}

			}

    //----------------------------------------------------------------------------------------

  Q=Arrptr->FHy[pq0]*dl  ;	// origional version included addition *dl but removed for compatability with UpdateH JCN
  // option to save V's
  if (Statesptr->voutput==ON)
  {
	  if (Q!=0)
	  {
	    hflow=getmax(zl+hl,zr+hr)-getmax(zl,zr);
        hflow=getmax(hflow,0);
        hflow=getmin(hflow,Solverptr->MaxHflow);
	    // calc Vy
	    Arrptr->Vy[pq0]=Arrptr->FHy[pq0]/hflow;
	    // get max Vy
	    Arrptr->maxVy[pq0]=getmax(Arrptr->maxVy[pq0],fabs(Arrptr->Vy[pq0]));
	  }
	  else Arrptr->Vy[pq0]=C(0.0);
  }
  return(Q);
}


//----------------------------------------------------------------------------

// Calculate net channel and floodplain flow in and out of domain boundary
void BoundaryFlux(States *Statesptr,Pars *Parptr,Solver *Solverptr,BoundCs *BCptr,ChannelSegmentType *ChannelSegments,Arrays *Arrptr, vector<ChannelSegmentType> *ChannelSegmentsVecPtr)
{
  int i,chseg;
  BCptr->Qin=C(0.0);BCptr->Qout=C(0.0);
  NUMERIC_TYPE qina=C(0.0), qinb=C(0.0), qinc=C(0.0), qind=C(0.0), qouta=C(0.0), qoutb=C(0.0), qoutc=C(0.0), qoutd=C(0.0);
  ChannelSegmentType *csp;

   // ***** Note ..... Qy is positive north to south

//#pragma omp parallel // private (Arrptr)
  {
	  //#pragma omp sections private (i)
	  {	 
  // North Boundary
  // loop through boundary and add positive Qy fluxes to Qin or subtract negative Qy fluxes from Qout
//#pragma omp section 
		  for(i=0;i<Parptr->xsz;i++) if(Arrptr->ChanMask[i]==-1)
  {
    if(Arrptr->Qy[i]>0) qina+=Arrptr->Qy[i];
    else qouta-=Arrptr->Qy[i];
  }
  // South boundary
  // loop through boundary and add positive Qy fluxes to Qout or subtract negative Qy fluxes from Qin
//#pragma omp section
  for(i=0;i<Parptr->xsz;i++) if(Arrptr->ChanMask[i+(Parptr->ysz-1)*Parptr->xsz]==-1)
  {
    if(Arrptr->Qy[i+Parptr->ysz*(Parptr->xsz+1)]>0) qoutb+=Arrptr->Qy[i+Parptr->ysz*(Parptr->xsz+1)];
    else qinb-=Arrptr->Qy[i+Parptr->ysz*(Parptr->xsz+1)];
  }
  // ***** Note ..... Qx is positive west to east

  //	West boundary
  // loop through boundary and add positive Qx fluxes to Qin or subtract negative Qx fluxes from Qout
//#pragma omp section
  for(i=0;i<Parptr->ysz;i++) if(Arrptr->ChanMask[i*Parptr->xsz]==-1) 
  {
    if(Arrptr->Qx[i*(Parptr->xsz+1)]>0) qinc+=Arrptr->Qx[i*(Parptr->xsz+1)];
    else qoutc-=Arrptr->Qx[i*(Parptr->xsz+1)];
  }

  // East boundary
  // loop through boundary and add positive Qx fluxes to Qout or subtract negative Qx fluxes from Qin
//#pragma omp section
  for(i=0;i<Parptr->ysz;i++) if(Arrptr->ChanMask[Parptr->xsz-1+i*Parptr->xsz]==-1)
  {
    if(Arrptr->Qx[Parptr->xsz+i*(Parptr->xsz+1)]>0) qoutd+=Arrptr->Qx[Parptr->xsz+i*(Parptr->xsz+1)];
    else qind-=Arrptr->Qx[Parptr->xsz+i*(Parptr->xsz+1)];
  }
	  }
  }

 BCptr->Qout = qouta + qoutb + qoutc + qoutd;
 BCptr->Qin = qina + qinb + qinc + qind;

  // Channel flows
  if(Statesptr->ChannelPresent==ON) 
  {
    BCptr->Qout+=BCptr->QChanOut;
    for(chseg=0;chseg<(int)ChannelSegmentsVecPtr->size();chseg++) // CCS
    {
      csp=ChannelSegments+chseg;
      for(i=0;i<csp->chsz;i++) 
      {
		  if (csp->Q_Ident[i] == QFIX4)
			BCptr->Qin+=csp->Q_Val[i]; // add up fixed flows QFIXs
		  else if (csp->Q_Ident[i] == QVAR5)
			BCptr->Qin+=InterpolateTimeSeries(csp->Q_TimeSeries[i],Solverptr->t);  // add up QVARs
      }
    }
  }


  // Point sources
  // account for positive point source in Qin and negative in Qout (MAT)
  BCptr->Qin += BCptr->Qpoint_pos;
  BCptr->Qout -= BCptr->Qpoint_neg;

  // calculate volume in and volume out
  BCptr->VolInMT += BCptr->Qin*Solverptr->Tstep;
  BCptr->VolOutMT += BCptr->Qout*Solverptr->Tstep;
 

  return;
}


//-----------------------------------------------------------------------------------


//---------------------------------------------------------------------------
// simple linear interpolation from a list of values provided as a boundary condition
// TimeSeries structure holds the previous matched index so that subsequent searches don't search over whole time series
//
// WARNING: InterpolateTimeSeries is not thread-safe
// because it modifies *timeSeries
NUMERIC_TYPE InterpolateTimeSeries(TimeSeries * timeSeries, NUMERIC_TYPE t)
{
	// if TimeSeries is reused, at same time point, 
	if (t == timeSeries->prev_time)
		return timeSeries->prev_value;

	const NUMERIC_TYPE * times = timeSeries->time;
	const NUMERIC_TYPE * values = timeSeries->value;
	
	// for values less than the start of the array - use 1st value
	if (t < times[0])
	{
		timeSeries->prev_time = t;
		timeSeries->prev_value = values[0];
		return values[0];
	}
	const int count = timeSeries->count;
	// for values greater than the end of the array - use last value
	if (t >= times[count - 1])
	{
		timeSeries->prev_time = t;
		timeSeries->prev_value = values[count - 1];
		return values[count - 1];
	}

	// if time is before the prev_index cursor, reset and start from begining
	// this shouldn't happen, just to ensure correcteness if t goes back in time
	// simply reset the search back to the start
	if (t < times[timeSeries->prev_index])
		timeSeries->prev_index = 0;


	NUMERIC_TYPE res = 0;
	for (int i = timeSeries->prev_index + 1; i < count; i++)
	{
		if (times[i - 1] <= t && times[i] > t)
		{
			NUMERIC_TYPE dt, a1, a2;
			dt = times[i] - times[i - 1];
			a1 = (t - times[i - 1]) / dt;
			a2 = C(1.0) - a1;

			res = a1*values[i] + a2*values[i - 1];
			timeSeries->prev_index = i - 1;
			break;
		}
	}

	timeSeries->prev_time = t;
	timeSeries->prev_value = res;

	return(res);
}

void drain_nodata_water
(
    Pars* Parptr,
    Solver* Solverptr,
    BoundCs* BCptr,
    Arrays* Arrptr
)
{
    if (Parptr->drain_nodata == OFF) return;

    NUMERIC_TYPE total_discharge = C(0.0);
#pragma omp parallel for reduction (+:total_discharge)
	for (int j=0; j<Parptr->ysz; j++)
	{
		for(int i=0; i<Parptr->xsz; i++)
		{
			NUMERIC_TYPE Z = Arrptr->DEM[j*Parptr->xsz + i];

            if (FABS(Z - Parptr->nodata_elevation) < 1e-6)
            {
			    NUMERIC_TYPE& H = Arrptr->H[j*Parptr->xsz + i];
                NUMERIC_TYPE Hold = H;
                H = C(0.0);

                total_discharge -= Hold*Parptr->dA/Solverptr->Tstep;
            }
        }
    }

    BCptr->Qout -= total_discharge;
    BCptr->VolOutMT += BCptr->Qout*Solverptr->Tstep;
}
