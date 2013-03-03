/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2013 Advanced Software Production Line, S.L.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 *  
 *  You may find a copy of the license under this software is released
 *  at COPYING file. This is LGPL software: you are welcome to develop
 *  proprietary applications using this library without any royalty or
 *  fee but returning back any change, improvement or addition in the
 *  form of source code, project image, documentation patches, etc.
 *
 *  For commercial support on build Websocket enabled solutions
 *  contact us:
 *          
 *      Postal address:
 *         Advanced Software Production Line, S.L.
 *         Edificio Alius A, Oficina 102,
 *         C/ Antonio Suarez Nº 10,
 *         Alcalá de Henares 28802 Madrid
 *         Spain
 *
 *      Email address:
 *         info@aspl.es - http://www.aspl.es/nopoll
 */
#ifndef __NOPOLL_HANDLERS_H__
#define __NOPOLL_HANDLERS_H__

/** 
 * @brief General async handler definition used to notify generic
 * events associated to a connection.
 *
 * Currently this handler is used by:
 * - \ref nopoll_listener_set_on_accept
 *
 * @param ctx The context where the wait is happening.
 *
 * @param conn The connection where the data or something meaningful
 * was detected.
 *
 * @param user_data Optional user data pointer defined by the user at
 * \ref nopoll_ctx_set_action_handler
 *
 * @return The function returns a boolean value which is interpreted
 * in an especific form according to the event.
 */
typedef nopoll_bool (*noPollActionHandler) (noPollCtx * ctx, noPollConn * conn, noPollPtr user_data);

/** 
 * @brief Handler used to define the create function for an IO mechanism.
 *
 * @param ctx The context where the io mechanism will be created.
 */
typedef noPollPtr (*noPollIoMechCreate)  (noPollCtx * ctx);

/** 
 * @brief Handler used to define the IO wait set destroy function for
 * an IO mechanism.
 *
 * @param ctx The context where the io mechanism will be destroyed.
 *
 * @param io_object The io object to be destroyed as created by \ref
 * noPollIoMechCreate handler.
 */
typedef void (*noPollIoMechDestroy)  (noPollCtx * ctx, noPollPtr io_object);

/** 
 * @brief Handler used to define the IO wait set clear function for an
 * IO mechanism.
 *
 * @param ctx The context where the io mechanism will be cleared.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler.
 */
typedef void (*noPollIoMechClear)  (noPollCtx * ctx, noPollPtr io_object);


/** 
 * @brief Handler used to define the IO wait function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef int (*noPollIoMechWait)  (noPollCtx * ctx, noPollPtr io_object);


/** 
 * @brief Handler used to define the IO add to set function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param conn The noPollConn to be added to the working set.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef nopoll_bool (*noPollIoMechAddTo)  (int               fds, 
					   noPollCtx       * ctx,
					   noPollConn      * conn,
					   noPollPtr         io_object);


/** 
 * @brief Handler used to define the IO is set function for an IO
 * mechanism.
 *
 * @param ctx The context where the io mechanism was created.
 *
 * @param conn The noPollConn to be added to the working set.
 *
 * @param io_object The io object to be created as created by \ref
 * noPollIoMechCreate handler where the wait will be implemented.
 */
typedef nopoll_bool (*noPollIoMechIsSet)  (noPollCtx       * ctx,
					   int               fds, 
					   noPollPtr         io_object);

/** 
 * @brief Handler used to define the foreach function that is used by
 * \ref nopoll_ctx_foreach_conn
 *
 * @param ctx The context where the foreach operation is taking place.
 *
 * @param conn The connection notified
 *
 * @param user_data Optional user defined pointer received at \ref
 * nopoll_ctx_foreach_conn.
 *
 * @return nopoll_true to stop the foreach process, otherwise
 * nopoll_false to keep checking the next connection until all
 * connections are notified.
 */
typedef nopoll_bool (*noPollForeachConn)  (noPollCtx  * ctx,
					   noPollConn * conn,
					   noPollPtr    user_data);

/** 
 * @brief Handler definition used to describe read functions used by \ref noPollConn.
 *
 * @param conn The connection where the readOperation will take place.
 *
 * @param buffer The buffer where data read from socket will be placed.
 *
 * @param buffer_size The buffer size that is receiving the function.
 */
typedef int (*noPollRead) (noPollConn * conn,
			   char       * buffer,
			   int          buffer_size);

/** 
 * @brief Handler definition used to notify websocket messages
 * received.
 *
 * This handler will be called when a websocket message is
 * received. Keep in mind the reference received on this handler will
 * be finished when the handler ends. If you need to have a reference
 * to the message after handler execution, acquire a reference via
 * \ref nopoll_msg_ref.
 *
 * @param ctx The context where the messagewas received.
 *
 * @param conn The connection where the message was received.
 *
 * @param msg The websocket message was received.
 *
 * @param user_data An optional user defined pointer.
 */
typedef void (*noPollOnMessageHandler) (noPollCtx  * ctx,
					noPollConn * conn,
					noPollMsg  * msg,
					noPollPtr  * user_data);

#endif
