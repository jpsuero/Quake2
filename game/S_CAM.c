#include "g_local.h"

//following qdevels tutorial

void ChasecamTrack(edict_t *ent);

//ent is owner of the chasecam
void ChasecamStart(edict_t* ent) 
{
	//this creates an temporary entitiy that we can manipulate
	edict_t* chasecam;

	//tell evrything that looks at toggle that chasecam is on and working
	ent->client->chasetoggle = 1;

	//make gun model non-existent
	ent->client->ps.gunindex = 0;

	chasecam = G_Spawn();
	chasecam->owner = ent;
	chasecam->solid = SOLID_NOT;
	chasecam->movetype = MOVETYPE_FLYMISSILE;

	//THIS TURNS OFF QUAKE 2'S INCINATION TO PREDICT CAMERA DIRECTION
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

	//this tells quake 2 to not send unnecessary info to other players
	ent->svflags |= SVF_NOCLIENT;

	//make angles of chasecam = player model angles
	VectorCopy(ent->s.angles, chasecam->s.angles);

	//clear entity size
	VectorClear(chasecam->mins);
	VectorClear(chasecam->maxs);

	//make chasecam origin same as player's
	VectorCopy(ent->s.origin, chasecam->s.origin);

	chasecam->classname = "chasecam";
	chasecam->prethink = ChasecamTrack;

	ent->client->chasecam = chasecam;
	ent->client->oldplayer = G_Spawn();
}

//ent is the chasecam entity
void ChasecamRestart(edict_t * ent)
{
	//keep thinking this function to check if player is out of the water
	//if player is dead, kill me and stop
	if (ent->owner->health <= 0)
	{
		G_FreeEdict(ent);
		return;
	}

	//if player is underwater break routine
	if (ent->owner->waterlevel)
	{
		return;
	}

	//if player is NOT underwater and not dead give cam back
	ChasecamStart(ent->owner);
	G_FreeEdict(ent);

}

//functions to remove cam and think when underwater
void ChasecamRemove(edict_t* ent, char* opt)
{
	//stop chasecam from moving
	VectorClear(ent->client->chasecam->velocity);

	//make weapon model of player appear on screen for first person
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	//make our invisible appearance the same model as display entity
	ent->s.modelindex = ent->client->oldplayer->s.modelindex;

	ent->svflags &= ~SVF_NOCLIENT;

	if (!strcmp(opt, "background"))
	{
		ent->client->chasetoggle = 0;
		G_FreeEdict(ent->client->chasecam);
		G_FreeEdict(ent->client->oldplayer);
		ent->client->chasecam = G_Spawn();
		ent->client->chasecam->owner = ent;
		ent->client->chasecam->solid = SOLID_NOT;
		ent->client->chasecam->movetype = MOVETYPE_FLYMISSILE;
		VectorClear(ent->client->chasecam->mins);
		VectorClear(ent->client->chasecam->maxs);
		ent->client->chasecam->classname = "chasecam";
		ent->client->chasecam->prethink = ChasecamRestart; // begin checking for emergence from the water
	}
	else if (!strcmp(opt, "off"))
	{
		// Added by WarZone - Begin
		if (ent->client->chasetoggle)
		{
			free(ent->client->oldplayer->client); // needed??
			G_FreeEdict(ent->client->oldplayer);
		}
		ent->client->chasetoggle = 0;
		G_FreeEdict(ent->client->chasecam);
		// Added by WarZone - End

	}
}

void ChasecamTrack(edict_t* ent)
{
	
	//temp vectors and trace variables
	trace_t tr;
	vec3_t spot1, spot2, dir;
	vec3_t forward, right, up;
	int distance;
	int tot;

	ent->nextthink = level.time + 100;
	//if owner underwater run remove routine
	if (ent->owner->waterlevel)
	{
		ChasecamRemove(ent, "background");
		return;
	}

	//get client angle and break it down into direction vectors
	AngleVectors(ent->owner->client->v_angle, forward, right, up);

	//go starting at the player's origin, forward, ent->chasedist1 distance, and save the location in vector spot2. not sure what this means
	VectorMA(ent->owner->s.origin, ent->chasedist1, forward, spot2);

	//make spot2 a bit higher, by adding 40 to Z coordinate
	spot2[2] = (spot2[2] + 40.000);

    //failed attempt to make spot2 further back from player
    // forward[2] = (forward[2] - 40.000);
    
    //this works! not perfect
    spot2[0] = (spot2[0] + 80.000);


	
    //if client is looking down, go backwards up in the air with a ratio of 0.6
	if (ent->owner->client->v_angle[0] < 0.000)
		VectorMA(spot2, -(ent->owner->client->v_angle[0] * 0.6), up, spot2);
	//same for client looking up
	else if (ent->owner->client->v_angle[0] > 0.000)
		VectorMA(spot2, (ent->owner->client->v_angle[0] * 0.6), up, spot2);

	//make tr traceline from player model's pos to spot 2
	tr = gi.trace(ent->owner->s.origin, NULL, NULL, spot2, ent->owner, false);

    // subtract the endpoint from the start point for length and direction manipulation 
    VectorSubtract(tr.endpos, ent->owner->s.origin, spot1);

    //in this case, length */

    ent->chasedist1 = VectorLength(spot1);



    /* go, starting from the end of the trace, 2 points forward (client

     * angles) and save the location in spot2 */

    VectorMA(tr.endpos, 2, forward, spot2);

    /* make spot1 the same for tempory vector modification and make spot1

     * a bit higher than spot2 */

    VectorCopy(spot2, spot1);

    spot1[2] += 32;



    /* another trace from spot2 to spot2, ignoring player, no masks */

    tr = gi.trace(spot2, NULL, NULL, spot1, ent->owner, false);



    /* if we hit something, copy the trace end to spot2 and lower spot2 */

    if (tr.fraction < 1.000)

    {

        VectorCopy(tr.endpos, spot2);

        spot2[2] -= 32;

    }



    /* subtract endpos spot2 from startpos the camera origin, saving it to

     * the dir vector, and normalize dir for a direction from the camera

     * origin, to the spot2 */

    VectorSubtract(spot2, ent->s.origin, dir);

    VectorNormalize(dir);



    /* subtract the same things, but save it in spot1 for a temporary

     * length calculation */

    VectorSubtract(spot2, ent->s.origin, spot1);

    distance = VectorLength(spot1);



    /* another traceline */

    tr = gi.trace(ent->s.origin, NULL, NULL, spot2, ent->owner, false);



    /* if we DON'T hit anyting, do some freaky stuff  */

    if (tr.fraction == 1.000)

    {



        /* subtract the endpos camera position, from the startpos, the

         * player, and save in spot1. Normalize spot1 for a direction, and

         * make that direction the angles of the chasecam for copying to the

         * clients view angle which is displayed to the client. (human) */

        VectorSubtract(ent->s.origin, ent->owner->s.origin, spot1);

        VectorNormalize(spot1);

        VectorCopy(spot1, ent->s.angles);



        /* calculate the percentages of the distances, and make sure we're

         * not going too far, or too short, in relation to our panning

         * speed of the chasecam entity */

        tot = (distance * 0.100);



        /* if we're going too fast, make us top speed */

        if (tot > 5.200)

        {

            ent->velocity[0] = ((dir[0] * distance) * 5.2);

            ent->velocity[1] = ((dir[1] * distance) * 5.2);

            ent->velocity[2] = ((dir[2] * distance) * 5.2);

        }

        else

        {



            /* if we're NOT going top speed, but we're going faster than

             * 1, relative to the total, make us as fast as we're going */



            if ((tot > 1.000))

            {

                ent->velocity[0] = ((dir[0] * distance) * tot);

                ent->velocity[1] = ((dir[1] * distance) * tot);

                ent->velocity[2] = ((dir[2] * distance) * tot);



            }

            else

            {



                /* if we're not going faster than one, don't accelerate our

                 * speed at all, make us go slow to our destination */



                ent->velocity[0] = (dir[0] * distance);

                ent->velocity[1] = (dir[1] * distance);

                ent->velocity[2] = (dir[2] * distance);



            }



        }



        /* subtract endpos;player position, from chasecam position to get

         * a length to determine whether we should accelerate faster from

         * the player or not */

        VectorSubtract(ent->owner->s.origin, ent->s.origin, spot1);



        if (VectorLength(spot1) < 20)

        {

            ent->velocity[0] *= 2;

            ent->velocity[1] *= 2;

            ent->velocity[2] *= 2;



        }



    }



    /* if we DID hit something in the tr.fraction call ages back, then

     * make the spot2 we created, the position for the chasecamera. */

    else

        VectorCopy(spot2, ent->s.origin);



    /* add to the distance between the player and the camera */

    
    ent->chasedist1 += 2;



    /* if we're too far away, give us a maximum distance */

    if (ent->chasedist1 > 60.00)

        ent->chasedist1 = 60.000;



    /* if we haven't gone anywhere since the last think routine, and we

     * are greater than 20 points in the distance calculated, add one to

     * the second chasedistance variable



     * The "ent->movedir" is a vector which is not used in this entity, so

     * we can use this a tempory vector belonging to the chasecam, which

     * can be carried through think routines. */

    if (ent->movedir == ent->s.origin)

    {

        if (distance > 20)

            ent->chasedist2++;

    }



    /* if we've buggered up more than 3 times, there must be some mistake,

     * so restart the camera so we re-create a chasecam, destroy the old one,

     * slowly go outwards from the player, and keep thinking this routing in

     * the new camera entity */

    if (ent->chasedist2 > 3)

    {

        ChasecamStart(ent->owner);

        G_FreeEdict(ent);

        return;

    }





    /* Copy the position of the chasecam now, and stick it to the movedir

      * variable, for position checking when we rethink this function */

    VectorCopy(ent->s.origin, ent->movedir);



}

//void Cmd_Chasecam_Toggle(edict_t* ent)
//{
    // Added by WarZone - Begin
  //  if (!ent->waterlevel && !ent->deadflag)
   // {
     //   if (ent->client->chasetoggle)
      //      ChasecamRemove(ent, "off");
       // else
      //      ChasecamStart(ent);
   // }
   // else if (ent->waterlevel && !ent->deadflag)
    //    gi.cprintf(ent, PRINT_HIGH, "Camera cannot be modified while in water\n");
    // Added by WarZone - End
//}

void CheckChasecam_Viewent(edict_t* ent)

{
    // Added by WarZone - Begin
    gclient_t* cl;

    if (!ent->client->oldplayer->client)
    {
        cl = (gclient_t*)malloc(sizeof(gclient_t));
        ent->client->oldplayer->client = cl;
    }
    // Added by WarZone - End

    if ((ent->client->chasetoggle == 1) && (ent->client->oldplayer))

    {

        ent->client->oldplayer->s.frame = ent->s.frame;
        /* Copy the origin, the speed, and the model angle, NOT

                      * literal angle to the display entity */

        VectorCopy(ent->s.origin, ent->client->oldplayer->s.origin);

        VectorCopy(ent->velocity, ent->client->oldplayer->velocity);

        VectorCopy(ent->s.angles, ent->client->oldplayer->s.angles);



        /* Make sure we are using the same model + skin as selected,

         * as well as the weapon model the player model is holding.

         * For customized deathmatch weapon displaying, you can

         * use the modelindex2 for different weapon changing, as you

         * can read in forthcoming tutorials */

         // Added by WarZone - Begin
        ent->client->oldplayer->s = ent->s; // copies over all of the important player related information
        // Added by WarZone - End

        gi.linkentity(ent->client->oldplayer);

    }



}