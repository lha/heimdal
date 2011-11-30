/*
 * Copyright (c) 2011 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "baselocl.h"

struct heim_db_type_data {
    const char *name;
    size_t size;
    bool (*dbopen)(void *, const char *, unsigned int);
    heim_data_t (*dbget)(void *ptr, heim_data_t, heim_error_t *);
    bool (*dbput)(void *ptr, heim_data_t, heim_data_t, heim_error_t *);
    bool (*dbsync)(void *ptr, heim_error_t *);
    void (*dbrelease)(void *ptr);
};

struct heim_db_type_data heim_db_sqlite = {
    "sqlite-simple",
    sizeof(sqlitelitedb),
    ....
};

struct heim_db {
    heim_db_type type;
    void *ptr;
}

static void
db_dealloc(void *ptr)
{
    heim_db_t db = ptr;
    if (db->type)
	db->type->dbrelease(ptr);
}

static int
db_cmp(void *a, void *b)
{
    return (unsigned long)a - (unsigned long)b;
}

static unsigned long
db_hash(void *ptr)
{
    return (unsigned long)ptr;
}

struct heim_type_data _heim_data_object = {
    HEIM_TID_DB,
    "db-object",
    NULL,
    db_dealloc,
    NULL,
    db_cmp,
    db_hash
};

/**
 * Create a data object
 *
 * @param string the string to create, must be an utf8 string
 *
 * @return string object
 */

heim_db_t
heim_db_create(heim_db_type type, const char *name, unsigned long flags)
{
    heim_db_t db;

    db = _heim_alloc_object(&_heim_db_object, sizeof(*db) + type->size);
    if (db) {
	db->type = type;
	db->ptr = (uint8_t *)db + sizeof(*db);
	if (db->type->dbopen(db->ptr, name, flags)) {
	    heim_release(db);
	    return NULL;
	}
    }
    return db;
}

/**
 * Return the type ID of data objects
 *
 * @return type id of string objects
 */

heim_tid_t
heim_db_get_type_id(void)
{
    return HEIM_TID_DB;
}

heim_db_t
heim_db_get(heim_db_t db, heim_data_t key, heim_error_t *error)
{
    return db->type->dbget(db->ptr, key, error);
}

bool
heim_db_put(heim_db_t db, heim_data_t key, heim_db_t value, heim_error_t *error)
{
    if (db->type->dbput == NULL)
	return false;
    return db->type->dbput(db->ptr, key, value, error);
}

bool
heim_db_sync(heim_db_t db, heim_error_t *error)
{
    if (db->type->dbsync)
	return db->type->dbsync(db->ptr, error);
    return true;
}
