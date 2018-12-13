/*
 * Copyright 2018 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WolkQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

QUEUE_ERR_T wolk_queue_init (wolk_queue *ctx)
{
    ctx->head = -1;
    ctx->tail = -1;
    return Q_FALSE;
}

QUEUE_ERR_T wolk_queue_push (wolk_queue *ctx, const char *reference, const char *command, const char *value){

    if (reference == NULL || command == NULL || value == NULL)
    {
        return Q_TRUE;
    }

    if (strlen(reference) >= REFERENCE_SIZE || strlen(command) >= COMMAND_SIZE || strlen(value)>=VALUE_SIZE)
    {
        return Q_TRUE;
    }

    if (ctx->head==QUEUE_SIZE-1)
    {
        return Q_TRUE;
    }

    if (ctx->head == -1)
    {
        ctx->tail = 0;
    }

    ctx->head++;

    memset (ctx->elem[ctx->head].command, 0, COMMAND_SIZE );
    memset (ctx->elem[ctx->head].reference, 0, REFERENCE_SIZE);
    memset (ctx->elem[ctx->head].value, 0, VALUE_SIZE);

    strcpy(ctx->elem[ctx->head].command, command );
    strcpy(ctx->elem[ctx->head].reference, reference );
    strcpy(ctx->elem[ctx->head].value, value );

    return Q_FALSE;
}

QUEUE_ERR_T wolk_queue_pop (wolk_queue *ctx, char *reference, char *command, char *value)
{
    int i=0;
    if (ctx->tail<0 || ctx->head<0)
    {
        return Q_TRUE;
    }

    strcpy(command, ctx->elem[ctx->tail].command);
    strcpy(reference, ctx->elem[ctx->tail].reference);
    strcpy(value, ctx->elem[ctx->tail].value);

    for (i=ctx->tail; i<ctx->head; i++)
    {
        memset (ctx->elem[i].command, 0, COMMAND_SIZE );
        memset (ctx->elem[i].reference, 0, REFERENCE_SIZE);
        memset (ctx->elem[i].value, 0, VALUE_SIZE);

        strcpy(ctx->elem[i].command, ctx->elem[i+1].command );
        strcpy(ctx->elem[i].reference, ctx->elem[i+1].reference );
        strcpy(ctx->elem[i].value, ctx->elem[i+1].value );
    }

    ctx->head--;


    return Q_FALSE;
}



QUEUE_ERR_T wolk_queue_clear (wolk_queue *ctx)
{

    ctx->head = -1;
    ctx->tail = -1;

    return Q_FALSE;
}
