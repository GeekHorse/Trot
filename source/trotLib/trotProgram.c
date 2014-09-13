/*
Copyright (c) 2014 Jeremiah Martell
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    - Neither the name of Jeremiah Martell nor the name of GeekHorse nor the
      name of Trot nor the names of its contributors may be used to endorse or
      promote products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/******************************************************************************/
/*!
	\file
	Implements the program functions.
*/
#undef  TROT_FILE_NUMBER
#define TROT_FILE_NUMBER 3

/******************************************************************************/
#include "trot.h"
#include "trotInternal.h"

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramLoad( TROT_INT memoryLimit, const char *savedProgram, TrotProgram **program_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( memoryLimit <= 0, TROT_RC_ERROR_PRECOND );
	ERR_IF( savedProgram == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( program_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*program_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramMemoryGetUsed( TrotProgram *program, TROT_INT *used )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( used == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*used) = program->memoryUsed;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramMemoryGetLimit( TrotProgram *program, TROT_INT *limit )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( limit == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	(*limit) = program->memoryLimit;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramMemorySetLimit( TrotProgram *program, TROT_INT limit )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( limit < 0, TROT_RC_ERROR_PRECOND );


	/* CODE */
	program->memoryLimit = limit;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramCyclesGet( TrotProgram *program, TROT_INT *cycles )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( cycles == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramCyclesSet( TrotProgram *program, TROT_INT cycles )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */
	(void)cycles;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramCyclesModify( TrotProgram *program, TROT_INT cycles )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */
	(void)cycles;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramStackAppendInt( TrotProgram *program, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */
	(void)n;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramStackAppendError( TrotProgram *program, TROT_INT tag )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */
	(void)tag;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramStackAppendString( TrotProgram *program, const char *string )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( string == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramStackPack( TrotProgram *program, TROT_INT n )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */
	(void)n;


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramRun( TrotProgram *program, TROT_INT *signalCode, TROT_INT *signalValue, TrotData **output_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( signalCode == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( signalValue == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( output_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*output_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramSave( TrotProgram **program_F, char **savedProgram_A )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */
	ERR_IF( program_F == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*program_F) == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( savedProgram_A == NULL, TROT_RC_ERROR_PRECOND );
	ERR_IF( (*savedProgram_A) != NULL, TROT_RC_ERROR_PRECOND );


	/* CODE */
	/* TODO */


	/* CLEANUP */
	cleanup:

	return rc;
}

/******************************************************************************/
/*!
	\brief 
	\param 
	\return TROT_RC
*/
TROT_RC trotProgramFree( TrotProgram **program_F )
{
	/* DATA */
	TROT_RC rc = TROT_RC_SUCCESS;


	/* PRECOND */


	/* CODE */
	if ( program_F == NULL || (*program_F) == NULL )
	{
		goto cleanup;
	}

	/* TODO lThreadList */
	TROT_HOOK_FREE( (*program_F) );
	(*program_F) = NULL;


	/* CLEANUP */
	cleanup:

	return rc;
}

