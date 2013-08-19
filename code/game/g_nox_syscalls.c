#include "g_local.h"

#undef trap_Trace
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );

void nox_trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) 
{
	int	i;
	int	j;
	int	restoreNum = 0;

	if ( passEntityNum >= 0 && passEntityNum < MAX_CLIENTS )
	{
		// Then, get a trace.
		trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );

		while ( results->fraction < 1.0 && results->entityNum >= 0 && results->entityNum < MAX_CLIENTS )
		{
			// The trace for the duelers
			if ((g_entities[passEntityNum].client->ps.duelInProgress || g_entities[results->entityNum].client->ps.duelInProgress) && 
			g_entities[results->entityNum].client->ps.clientNum != g_entities[passEntityNum].client->ps.duelIndex )
			{
				restoreNum |= (1 << passEntityNum);
				restoreNum |= (1 << results->entityNum);
				g_entities[results->entityNum].r.contents = ~contentmask;
				g_entities[passEntityNum].r.contents = ~contentmask;
			}
			else
			{
				break;
			}
			
			// Trace again, and again, and again untill we hit something to stop this loop (ground, real enemy?)
			trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
		}

		// Finalize!
		trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );

		if ( restoreNum > 0 )
		{
			// Restore all contents! They aren't blanco for real!
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				if (g_entities[i].inuse && g_entities[i].client && 
				(restoreNum & (1 << i)) && g_entities[i].client->ps.pm_type != PM_DEAD)
				{
					g_entities[i].r.contents |= contentmask;
				}
			}
		}
	}
	// If it aint a modified version of tracing, just do the ordinary trace.
	else
	{
		trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
	}
}
