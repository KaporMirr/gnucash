/********************************************************************\
 * Backend.h -- private api for engine Backend                      *
 *                                                                  *
 * Copyright (c) 2000, 2001 Linas Vepstas <linas@linas.org>         *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/* 
 * FILE:
 * BackendP.h
 *
 * FUNCTION:
 * Pseudo-object defining how the engine can interact with different
 * back-ends (which may be SQL databases, or network interfaces to 
 * remote GnuCash servers.  In theory, file-io should be a type of 
 * backend).
 * 
 * The callbacks will be called at the appropriate times during 
 * a book session to allow the backend to store the data as needed.
 *
 */

#ifndef __XACC_BACKEND_P_H__
#define __XACC_BACKEND_P_H__

#include "config.h"

#include "Account.h"
#include "Backend.h"
#include "Group.h"
#include "Query.h"
#include "Transaction.h"
#include "gnc-book.h"
#include "gnc-pricedb.h"

/*
 * The book_begin() routine gives the backend a second initialization
 *    opportunity.  It is suggested that the backend check that 
 *    the URL is syntactically correct, and that it is actually
 *    reachable.  This is probably(?) a good time to initialize
 *    the actual network connection.
 *
 *    The 'ignore_lock' argument indicates whether the single-user
 *    lock on the backend should be cleared.  The typical GUI sequence
 *    leading to this is: (1) GUI attempts to open the backend
 *    by calling this routine with FALSE==ignore_lock.  (2) If backend  
 *    error'ed BACKEND_LOCK, then GUI asks user what to do. (3) if user 
 *    answers 'break & enter' then this routine is called again with
 *    TRUE==ignore_lock.
 *
 *    The 'create_if_nonexistent' argument indicates whether this
 *    routine should create a new 'database', if it doesn't already
 *    exist. For example, for a file-backend, this would create the
 *    file, if it didn't already exist.  For an SQL backend, this
 *    would create the database (the schema) if it didn't already 
 *    exist.  This flag is used to implement the 'SaveAs' GUI, where
 *    the user requests to save data to a new backend.
 *
 * The book_load() routine should return at least an account tree,
 *    and all currencies.  It does not have to return any transactions
 *    whatsoever, as these are obtained at a later stage when a user
 *    opens a register, resulting in a query being sent to the backend.
 *
 *    (Its OK to send over transactions at this point, but one should 
 *    be careful of the network load; also, its possible that whatever 
 *    is sent is not what the user wanted anyway, which is why its 
 *    better to wait for the query).
 *
 * The trans_commit_edit() routine takes two transaction arguments:
 *    the first is the proposed new transaction; the second is the
 *    'original' transaction. The second argument is here for 
 *    convenience; it had better be substantially equivalent to
 *    the argument for the trans_begin_edit() callback.  (It doesn't
 *    have to be identical, it can be a clone).
 *
 * The trans_rollback_edit() routine is invoked in one of two different
 *    ways.  In one case, the user may hit 'undo' in the GUI, resulting
 *    in xaccTransRollback() being called, which in turn calls this
 *    routine.  In this manner, xaccTransRollback() implements a
 *    single-level undo convenience routine for the GUI.  The other
 *    way in which this routine gets invoked involves conflicting
 *    edits by two users to the same transaction.  The second user
 *    to make an edit will typically fail in trans_commit_edit(),
 *    with trans_commit_edit() returning an error code.  This 
 *    causes xaccTransCommitEdit() to call xaccTransRollback()
 *    which in turn calls this routine.  Thus, this routine
 *    gives the backend a chance to clean up failed commits.
 *
 *    If the second user tries to modify a transaction that
 *    the first user deleted, then the backend should set the error
 *    to ERR_BACKEND_MOD_DESTROY from this routine, so that the 
 *    engine can properly clean up.
 *
 * The run_query() callback takes a GnuCash query object. 
 *    For an SQL backend,  the contents of the query object need to 
 *    be turned into a corresponding SQL query statement, and sent 
 *    to the database for evaluation. The database will return a 
 *    set of splits and transactions, and this callback needs
 *    to poke these into the account-group hierarchy held by the 
 *    query object. 
 *
 *    For a network-communications backend, essentially the same is 
 *    done, except that this routine would convert the query to wire 
 *    protocol, get an answer from the remote server, and push that
 *    into the account-group object.
 *
 *    Note a peculiar design decision we've used here. The query
 *    callback has returned a list of splits; these could be returned
 *    directly to the caller. They are not.  By poking them into the
 *    existing account hierarchy, we are essentially building a local
 *    cache of the split data.  This will allow the GnuCash client to 
 *    continue functioning even when disconnected from the server:
 *    this is because it will have its local cache of data to work from.
 *
 * The sync() routine synchronizes the engine contents to the backend.
 *    This is done by using version numbers (hack alert -- the engine
 *    does not currently contain version numbers).
 *    If the engine contents are newer than what's in the backend, the 
 *    data is stored to the backend.  If the engine contents are older,
 *    then the engine contents are updated.  
 *
 *    Note that this sync operation is only meant to apply to the 
 *    current contents of the engine.  This routine is not intended
 *    to be used to fetch account/transaction data from the backend.
 *    (It might pull new splits from the backend, if this is what is
 *    needed to update an existing transaction.  It might pull new 
 *    currencies (??))
 *
 * The events_pending() routines should return true if there are
 *    external events which need to be processed to bring the
 *    engine up to date with the backend.
 *
 * The process_events() routine should process any events indicated
 *    by the events_pending() routine. It should return TRUE if
 *    the engine was changed while engine events were suspended.
 *
 * The last_err member indicates the last error that occurred.
 *    It should probably be implemented as an array (actually,
 *    a stack) of all the errors that have occurred.
 */

typedef struct _backend Backend;

struct _backend 
{
  void (*book_begin) (GNCBook *, const char *book_id, 
                      gboolean ignore_lock, gboolean create_if_nonexistent);
  AccountGroup * (*book_load) (Backend *);
  GNCPriceDB * (*price_load) (Backend *);
  void (*book_end) (Backend *);

  void (*account_begin_edit) (Backend *, Account *);
  void (*account_commit_edit) (Backend *, Account *);
  void (*trans_begin_edit) (Backend *, Transaction *);
  void (*trans_commit_edit) (Backend *, Transaction *new, Transaction *orig);
  void (*trans_rollback_edit) (Backend *, Transaction *);

  void (*price_begin_edit) (Backend *, GNCPrice *);
  void (*price_commit_edit) (Backend *, GNCPrice *);

  void (*run_query) (Backend *, Query *);
  void (*price_lookup) (Backend *, GNCPriceLookup *);
  void (*sync) (Backend *, AccountGroup *);
  void (*sync_price) (Backend *, GNCPriceDB *);

  gboolean (*events_pending) (Backend *be);
  gboolean (*process_events) (Backend *be);

  GNCBackendError last_err;
};

/*
 * The xaccBackendSetError() routine pushes an error code onto the error
 *   stack. (FIXME: the stack is 1 deep in current implementation).
 *
 * The xaccBackendGetError() routine pops an error code off the error
 *   stack.
 */

void xaccBackendSetError (Backend *be, GNCBackendError err);
GNCBackendError xaccBackendGetError (Backend *be);

/*
 * The xaccGetAccountBackend() subroutine will find the
 *    persistent-data storage backend associated with this account.
 *    This routine traverses up the account hierarchy until it
 *    finds and account-group node that has a backend associated with
 *    it.  The assumption is that all accounts in that account-group
 *    share a common back-end.
 *
 * The xaccGetTransactionBackend() subroutine does the same, for a given
 *    transaction.
 */

Backend * xaccAccountGetBackend (Account *account);
Backend * xaccTransactionGetBackend (Transaction *trans);

/*
 * The xaccGroupSetBackend() associates a backend to a group
 */
void xaccGroupSetBackend (AccountGroup *group, Backend *be);
Backend * xaccGroupGetBackend (AccountGroup *group);
Backend * xaccGNCBookGetBackend (GNCBook *book);

/* 
 * Put a link to the backend that handles this pricedb 
 */
void xaccPriceDBSetBackend (GNCPriceDB *prdb, Backend *be);
Backend * xaccPriceDBGetBackend (GNCPriceDB *prdb);

#endif /* __XACC_BACKEND_P_H__ */