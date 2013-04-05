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
#include <nopoll.h>

nopoll_bool debug = nopoll_false;

nopoll_bool test_sending_and_check_echo (noPollConn * conn, const char * label, const char * msg)
{
	char  buffer[1024];
	int   length = strlen (msg);
	int   bytes_read;

	/* send content text(utf-8) */
	printf ("%s: sending content..\n", label);
	if (nopoll_conn_send_text (conn, msg, length) != length) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 1024, blocking and with a 3 seconds timeout) */
	bytes_read = nopoll_conn_read (conn, buffer, length, nopoll_true, 3000);
	if (bytes_read > 0)
		buffer[bytes_read] = 0;
	
	if (bytes_read != length) {
		printf ("ERROR: expected to find 14 bytes but found %d..\n", bytes_read);
		return nopoll_false;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp (buffer, msg)) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			buffer);
		return nopoll_false;
	} /* end if */

	printf ("%s: received reply and echo matches..\n", label);

	/* return that we sent and received the echo reply */
	return nopoll_true;
}

noPollCtx * create_ctx (void) {
	
	/* create a context */
	noPollCtx * ctx = nopoll_ctx_new ();
	nopoll_log_enable (ctx, debug);
	nopoll_log_color_enable (ctx, debug);
	return ctx;
}

nopoll_bool test_01_strings (void) {
	/* check string compare functions */
	if (! nopoll_ncmp ("GET ", "GET ", 4)) {
		printf ("ERROR (1): expected to find right equal comparison..\n");
		return nopoll_false;
	}

	if (! nopoll_ncmp ("GET VALUE", "GET ", 4)) {
		printf ("ERROR (2): expected to find right equal comparison..\n");
		return nopoll_false;
	}

	return nopoll_true;
}

nopoll_bool test_01_base64 (void) {
	char buffer[1024];
	int  size = 1024;
	int  iterator = 0;

	/* call to produce base 64 (we do a loop to ensure we don't
	 * leak through openssl (220) bytes */
	while (iterator < 10) {
		size = 1024;
		if (! nopoll_base64_encode ("This is a test", 14, buffer, &size)) {
			printf ("ERROR: failed to encode this is a test..\n");
			return nopoll_false;
		} /* end if */
		
		/* check result */
		if (! nopoll_cmp (buffer, "VGhpcyBpcyBhIHRlc3Q=")) {
			printf ("ERROR: expected to find encoded base64 string %s but found %s..\n", 
				"VGhpcyBpcyBhIHRlc3Q=", buffer);
			return nopoll_false;
		}

		iterator++;
	}

	/* now decode content */
	iterator = 0;
	while (iterator < 10) {
		size = 1024;
		if (! nopoll_base64_decode ("VGhpcyBpcyBhIHRlc3Q=", 20, buffer, &size)) {
			printf ("ERROR: failed to decode base64 content..\n");
		}
		
		/* check result */
		if (! nopoll_cmp (buffer, "This is a test")) {
			printf ("ERROR: expected to find encoded base64 string %s but found %s..\n", 
				"This is a test", buffer);
			return nopoll_false;
		} /* end if */

		iterator++;
	}

	
	return nopoll_true;
}

nopoll_bool test_01_masking (void) {

	char         mask[4];
	int          mask_value;
	char         buffer[1024];
	noPollCtx  * ctx;

	/* create context */
	ctx = create_ctx ();

	mask_value = random ();
	printf ("Test-01 masking: using masking value %d\n", mask_value);
	nopoll_set_32bit (mask_value, mask);

	memcpy (buffer, "This is a test value", 20);
	nopoll_conn_mask_content (ctx, buffer, 20, mask);

	if (nopoll_ncmp (buffer, "This is a test value", 20)) {
		printf ("ERROR: expected to find different values after masking but found the same..\n");
		return nopoll_false;
	}

	/* revert changes */
	nopoll_conn_mask_content (ctx, buffer, 20, mask);

	if (! nopoll_ncmp (buffer, "This is a test value", 20)) {
		printf ("ERROR: expected to find SAME values after masking but found the same..\n");
		return nopoll_false;
	} /* end if */

	/* now check transfering these values to the mask */
	if (nopoll_get_32bit (mask) != mask_value) {
		printf ("ERROR: found failure while reading the mask from from buffer..\n");
		return nopoll_false;
	}
	printf ("Test 01 masking: found mask in the buffer %d == %d\n", 
		nopoll_get_32bit (mask), mask_value);

	nopoll_ctx_unref (ctx);
	return nopoll_true;
}

nopoll_bool test_01 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 1) {
		printf ("ERROR: expected to find 1 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	/* ensure connection status is ok */
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR (3): expected to find proper connection status, but found failure..\n");
		return nopoll_false;
	}

	printf ("Test 01: reference counting for the connection: %d\n", nopoll_conn_ref_count (conn));

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4): expected to find proper connection handshake finished, but found it is still not prepared..\n");
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_02 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	noPollMsg  * msg;
	int          iter;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 02: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply */
	iter = 0;
	while ((msg = nopoll_conn_get_msg (conn)) == NULL) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR: received websocket connection close during wait reply..\n");
			return nopoll_false;
		}

		nopoll_sleep (10000);

		if (iter > 10)
			break;
	} /* end if */

	/* check content received */
	if (! nopoll_cmp (nopoll_msg_get_payload (msg), "This is a test")) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			(const char *) nopoll_msg_get_payload (msg));
		return nopoll_false;
	} /* end if */

	/* unref message */
	nopoll_msg_unref (msg);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_03 (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 03: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "This is a test", 14) != 14) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 1024, blocking and with a 3 seconds timeout) */
	printf ("Test 03: now reading reply..\n");
	bytes_read = nopoll_conn_read (conn, buffer, 14, nopoll_true, 3000);
	
	if (bytes_read != 14) {
		printf ("ERROR: expected to find 14 bytes but found %d..\n", bytes_read);
		return nopoll_false;
	} /* end if */

	/* check content received */
	if (! nopoll_ncmp (buffer, "This is a test", 14)) {
		printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
			buffer);
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_04 (int chunk_size) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;
	FILE       * file;
	struct stat  stat_buf;
	int          total_read = 0;

	/* create context */
	ctx = create_ctx ();

	/* check connections registered */
	if (nopoll_ctx_conns (ctx) != 0) {
		printf ("ERROR: expected to find 0 registered connections but found: %d\n", nopoll_ctx_conns (ctx));
		return nopoll_false;
	} /* end if */

	nopoll_ctx_unref (ctx);

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 02: sending basic content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, "get-file", 8) != 8) {
		printf ("ERROR: Expected to find proper send operation..\n");
		return nopoll_false;
	}

	file = fopen ("tmp", "w");
	if (file == NULL) {
		printf ("ERROR: unable to open file tmp for content comparision\n");
		return nopoll_false; 
	} /* end if */

	stat ("nopoll-regression-client.c", &stat_buf);

	while (total_read < stat_buf.st_size) {
		/* wait for the reply (try to read 1024, blocking) */
		bytes_read = nopoll_conn_read (conn, buffer, chunk_size, nopoll_true, 100);
		/* printf ("Test 04: read %d bytes over the connection %d\n", bytes_read, nopoll_conn_get_id (conn));  */

		if (bytes_read < 0) {
			printf ("ERROR: expected to find bytes from the connection but found: %d\n", bytes_read);
			return nopoll_false;
		}

		if (bytes_read == 0) {
			/* printf ("Test 04: nothing found (0 bytes), total read %d, total requested: %ld\n", total_read, stat_buf.st_size); */
			continue;
		}

		/* write content */
		fwrite (buffer, 1, bytes_read, file);
	
		/* count total read bytes */
		total_read += bytes_read;

	} /* end while */
	fclose (file);

	/* now check both files */
	printf ("Test 04: checking content download (chunk_size=%d)...\n", chunk_size);
	if (system ("diff nopoll-regression-client.c tmp > /dev/null")) {
		printf ("ERROR: failed to download file from server, content differs. Check: diff nopoll-regression-client.c tmp\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_04a (void) {
	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          result;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* attempt to read without blocking */
	printf ("Test 04-a: checking non-blocking API..\n");
	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 0);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}
		
	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 300);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);

	result = nopoll_conn_read (conn, buffer, 1024, nopoll_false, 1000);
	if (result != -1) {
		printf ("ERROR: expected return result -1(%d)\n", result);
		return nopoll_false;
	}

	printf ("Test 04-a: ok, operation not blocked, result %d\n", result);

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);
	

	return nopoll_true;
}

nopoll_bool test_05 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;
	char         buffer[1024];
	int          bytes_read;
	const char * msg = " klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw";

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	printf ("Test 05: sending UTF-8 content..\n");

	/* send content text(utf-8) */
	if (nopoll_conn_send_text (conn, msg, -1) <= 0) {
		printf ("ERROR: Expected to find proper send operation (nopoll_conn_send_test) returned less or 0..\n");
		return nopoll_false;
	}

	/* wait for the reply (try to read 322, blocking and with a 3 seconds timeout) */
	bytes_read = nopoll_conn_read (conn, buffer, 322, nopoll_true, 3000);
	if (bytes_read != 322) {
		printf ("ERROR: expected to receive 322 bytes, but received %d\n", bytes_read);
		return nopoll_false;
	}

	if (! nopoll_ncmp (buffer, msg, 322)) {
		printf ("ERROR: expected to receive another content....\n");
		printf ("Expected: %s\n", msg);
		printf ("Received: %s\n", buffer);

		return nopoll_false;
	}

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_06 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, NULL, "localhost", "1235", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4): expected to find proper connection handshake finished, but found it is still not prepared..\n");
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	if (! nopoll_conn_is_tls_on (conn)) {
		printf ("ERROR (5): expected to find TLS enabled on the connection but found it isn't..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_07 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to create a connection */
	conn = nopoll_conn_tls_new (ctx, NULL, "localhost", "1235", NULL, NULL, NULL, NULL);
	if (! nopoll_conn_is_ok (conn)) {
		printf ("ERROR: Expected to find proper client connection status, but found error..\n");
		return nopoll_false;
	}

	/* check if the connection already finished its connection
	   handshake */
	while (! nopoll_conn_is_ready (conn)) {

		if (! nopoll_conn_is_ok (conn)) {
			printf ("ERROR (4): expected to find proper connection handshake finished, but found it is still not prepared..\n");
			return nopoll_false;
		} /* end if */

		/* wait a bit 10ms */
		nopoll_sleep (10000);
	} /* end if */

	printf ("Test 07: testing sending TLS content over the wire..\n");
	if (! test_sending_and_check_echo (conn, "Test 07", "This is a test"))
		return nopoll_false;

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_08 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to connect to TLS port expecting non-TLS protocol */
	conn = nopoll_conn_new (ctx, "localhost", "1235", NULL, NULL, NULL, NULL);

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected to FAILING connection status, but found error..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_09 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* setup the protocol version to see how it breaks (it should) */
	nopoll_ctx_set_protocol_version (ctx, 12);

	/* call to connect to TLS port expecting non-TLS protocol */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected to FAILING connection status due to protocol version error, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_10 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* call to connect from an origining that shouldn't be allowed */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, "http://deny.aspl.es");

	/* wait a bit 100ms */
	nopoll_sleep (100000);

	if (nopoll_conn_is_ready (conn)) {
		printf ("ERROR: Expected to FAILING connection status due to origing denied, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish connection */
	nopoll_conn_close (conn);
	
	/* finish */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}

nopoll_bool test_11 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;

	/* reinit again */
	ctx = create_ctx ();

	/* create a working connection */
	conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);

	if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
		printf ("ERROR: Expected to FAILING connection status due to origing denied, but it working..\n");
		return nopoll_false;
	} /* end if */

	/* finish */
	nopoll_ctx_unref (ctx);

	/* finish connection */
	nopoll_conn_close (conn);
	
	return nopoll_true;
}

nopoll_bool test_12 (void) {

	noPollCtx  * ctx;
	noPollConn * conn;
	int          iterator;

	/* time tracking */
	struct  timeval    start;
	struct  timeval    stop;
	struct  timeval    diff;


	/* reinit again */
	ctx = create_ctx ();

	/* start */
	gettimeofday (&start, NULL);

	iterator = 0;
	while (iterator < 1000) {
		/* create a working connection */
		conn = nopoll_conn_new (ctx, "localhost", "1234", NULL, NULL, NULL, NULL);
		
		if (! nopoll_conn_wait_until_connection_ready (conn, 5)) {
			printf ("ERROR: Expected to FAILING connection status due to origing denied, but it working..\n");
			return nopoll_false;
		} /* end if */

		/* finish connection */
		nopoll_conn_close (conn);

		iterator++;
	} /* end while */

	/* finish */
	nopoll_ctx_unref (ctx);

	/* stop */
	gettimeofday (&stop, NULL);

	nopoll_timeval_substract (&stop, &start, &diff);

	printf ("Test 12: created %d connections in %ld.%ld secs\n", 
		iterator, diff.tv_sec, diff.tv_usec);
	
	
	return nopoll_true;
}

nopoll_bool test_13_test (noPollCtx * ctx, const char * serverName, const char * _certificateFile, const char * _privateKey)
{
	const char * certificateFile;
	const char * privateKey;

	if (! nopoll_ctx_find_certificate (ctx, serverName, NULL, NULL, NULL)) {
		printf ("Test 13: it SHOULD find something about found.server.com but function reported failure status..\n");
		return nopoll_false;
	}

	if (! nopoll_ctx_find_certificate (ctx, serverName, &certificateFile, &privateKey, NULL)) {
		printf ("Test 13: it SHOULD find something about found.server.com but function reported failure status..\n");
		return nopoll_false;
	}

	if (! nopoll_cmp (certificateFile, _certificateFile)) {
		printf ("Test 13: expected to find certificate %s, but found %s\n", _certificateFile, certificateFile);
		return nopoll_false;
	}
	if (! nopoll_cmp (privateKey, _privateKey)) {
		printf ("Test 13: expected to find certificate %s, but found %s\n", _privateKey, privateKey);
		return nopoll_false;
	}
	return nopoll_true;
}

nopoll_bool test_13 (void)
{
	noPollCtx * ctx;

	/* create ctx */
	ctx = nopoll_ctx_new ();

	if (nopoll_ctx_find_certificate (ctx, "not-found", NULL, NULL, NULL)) {
		printf ("Test 13: it shouldn't find anything but function reported ok status..\n");
		return nopoll_false;
	}

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "found.server.com", "test.crt", "test.key", NULL)) {
		printf ("Test 13: unable to install certificate...\n");
		return nopoll_false;
	} /* end if */

	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "another.server.com", "another.test.crt", "another.test.key", NULL)) {
		printf ("Test 13: unable to install certificate (another.server.com)...\n");
		return nopoll_false;
	} /* end if */


	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "another.server.com", "another.test.crt", "another.test.key")) 
		return nopoll_false;

	/* register */
	if (! nopoll_ctx_set_certificate (ctx, "other.server.com", "other.test.crt", "other.test.key", NULL)) {
		printf ("Test 13: unable to install certificate (another.server.com)...\n");
		return nopoll_false;
	} /* end if */

	if (! test_13_test (ctx, "found.server.com", "test.crt", "test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "another.server.com", "another.test.crt", "another.test.key")) 
		return nopoll_false;

	if (! test_13_test (ctx, "other.server.com", "other.test.crt", "other.test.key")) 
		return nopoll_false;

	/* release ctx */
	nopoll_ctx_unref (ctx);

	return nopoll_true;
}



int main (int argc, char ** argv)
{
	int iterator;

	printf ("** NoPoll: Websocket toolkit (regression test).\n");
	printf ("** Copyright (C) 2013 Advanced Software Production Line, S.L.\n**\n");
	printf ("** NoPoll regression tests: version=%s\n**\n",
		VERSION);
	printf ("** To gather information about time performance you can use:\n**\n");
	printf ("**     >> time ./nopoll-regression-client [--debug]\n**\n");
	printf ("** To gather information about memory consumed (and leaks) use:\n**\n");
	printf ("**     >> libtool --mode=execute valgrind --leak-check=yes --error-limit=no ./nopoll-regression-client\n**\n");
	printf ("**\n");
	printf ("** Report bugs to:\n**\n");
	printf ("**     <info@aspl.es> noPoll mailing list\n**\n");

	iterator = 1;
	while (iterator < argc) {
		/* check for debug */
		printf ("Checking agument: %s\n", argv[iterator]);
		if (nopoll_cmp (argv[iterator], "--debug")) {
			printf ("Activating debug..\n");
			debug = nopoll_true;
		} /* end if */

		/* next position */
		iterator++;
	}

	printf ("INFO: starting tests with pid: %d\n", getpid ());

	if (test_01_strings ()) {
		printf ("Test 01-strings: Library strings support [   OK   ]\n");
	}else {
		printf ("Test 01-strings: Library strings support [ FAILED ]\n");
		return -1;
	}

	if (test_01_base64 ()) {
		printf ("Test 01-bas64: Library bas64 support [   OK   ]\n");
	}else {
		printf ("Test 01-bas64: Library bas64 support [ FAILED ]\n");
		return -1;
	}

	if (test_01_masking ()) {
		printf ("Test 01-masking: Library websocket content masking support [   OK   ]\n");
	}else {
		printf ("Test 01-masking: Library websocket content masking support [ FAILED ]\n");
		return -1;
	}

	if (test_01 ()) {	
		printf ("Test 01: Simple connect and disconnect [   OK   ]\n");
	}else {
		printf ("Test 01: Simple connect and disconnect [ FAILED ]\n");
		return -1;
	}

	if (test_02 ()) {	
		printf ("Test 02: Simple request/reply [   OK   ]\n");
	}else {
		printf ("Test 02: Simple request/reply [ FAILED ]\n");
		return -1;
	}

	/* test streaming api */
	if (test_03 ()) {	
		printf ("Test 03: test streaming api [   OK   ]\n");
	}else {
		printf ("Test 03: test streaming api [ FAILED ]\n");
		return -1;
	}

	if (test_04 (1024)) {	
		printf ("Test 04: test streaming api (II) [   OK   ]\n");
	}else {
		printf ("Test 04: test streaming api (II) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (512)) {	
		printf ("Test 04-a: test streaming api (III) [   OK   ]\n");
	}else {
		printf ("Test 04-a: test streaming api (III) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (137)) {	
		printf ("Test 04-b: test streaming api (IV) [   OK   ]\n");
	}else {
		printf ("Test 04-b: test streaming api (IV) [ FAILED ]\n");
		return -1;
	}

	if (test_04 (17)) {	
		printf ("Test 04-c: test streaming api (V) [   OK   ]\n");
	}else {
		printf ("Test 04-c: test streaming api (V) [ FAILED ]\n");
		return -1;
	}

	if (test_04a ()) {
		printf ("Test 04-a: check non-blocking streaming and message based API  [   OK   ]\n");
	} else {
		printf ("Test 04-a: check non-blocking streaming and message based API [ FAILED ]\n");
		return -1;
	}

	if (test_05 ()) {
		printf ("Test 05: sending utf-8 content [   OK   ]\n");
	} else {
		printf ("Test 05: sending utf-8 content [ FAILED ]\n");
		return -1;
	}

	if (test_06 ()) {
		printf ("Test 06: testing basic TLS connect [   OK   ]\n");
	} else {
		printf ("Test 06: testing basic TLS connect [ FAILED ]\n");
		return -1;
	}

	if (test_07 ()) {
		printf ("Test 07: testing TLS request/reply [   OK   ]\n");
	} else {
		printf ("Test 07: testing TLS request/reply [ FAILED ]\n");
		return -1;
	}

	if (test_08 ()) {
		printf ("Test 08: test normal connect to TLS port [   OK   ]\n");
	} else {
		printf ("Test 08: test normal connect to TLS port [ FAILED ]\n");
		return -1;
	}

	if (test_09 ()) {
		printf ("Test 09: ensure we only support Sec-WebSocket-Version: 13 [   OK   ]\n");
	} else {
		printf ("Test 09: ensure we only support Sec-WebSocket-Version: 13 [ FAILED ]\n");
		return -1;
	}

	if (test_10 ()) {
		printf ("Test 10: test checking origing in on open and denying it [   OK   ]\n");
	} else {
		printf ("Test 10: test checking origing in on open and denying it [ FAILED ]\n");
		return -1;
	}

	if (test_11 ()) {
		printf ("Test 11: release context after connection [   OK   ]\n");
	} else {
		printf ("Test 11: release context after connection [ FAILED ]\n");
		return -1;
	}

	if (test_12 ()) {
		printf ("Test 12: create huge amount of connections in a short time [   OK   ]\n");
	} else {
		printf ("Test 12: create huge amount of connections in a short time [ FAILED ]\n");
		return -1;
	}
	
	if (test_13 ()) {
		printf ("Test 13: testing certificate storege [   OK    ]\n");
	} else {
		printf ("Test 13: testing certificate storege [ FAILED  ]\n");
		return -1;
	}

	/* send a frame with few content as indicated by the payload
	 * header (fragmented and unfragmented) */

	/* add support to reply with redirect 301 to an opening
	 * request: page 19 and 22 */

	/* add support for basic HTTP auth before proceding with the
	 * handshake. The the possibility to use htpasswd tools. Page 19 and 22 */

	/* add support to define cookies by the server: page 20 */

	/* update the library to split message frames into smaller
	 * complete frames when bigger messages are received. */

	/* add support for proxy mode */

	/* check control files aren't flagged as fragmented */
	
	/* upload a file to the server ...*/

	/* more streaming api testing, get bigger content as a
	 * consequence of receiving several messages */

	/* test streaming API when it timeouts */

	/* test sending wrong mime headers */

	/* test sending missing mime headers */

	/* test injecting wrong bytes */

	/* test sending lot of MIME headers (really lot of
	 * information) */

	/* test checking protocols and denying it */

	/* test sending ping */

	/* test sending pong (without ping) */

	/* test sending frames 126 == ( 65536) */

	/* test sending frames 127 == ( more than 65536) */

	/* test applying limits to incoming content */

	/* test splitting into several frames content bigger */

	/* test wrong UTF-8 content received on text frames */

	/* add support to sending close frames with status code and a
	 * textual indication as defined by page 36 */

	/* call to cleanup */
	nopoll_cleanup_library ();
	printf ("All tests ok!!\n");


	return 0;
}

/* end-of-file-found */
