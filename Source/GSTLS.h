/** Interface for GSTLS classes for GNUStep
   Copyright (C) 2012 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2101

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   */

#import "Foundation/NSObject.h"

@class  NSDate;
@class  NSDictionary;
@class  NSHost;
@class  NSString;

#if     defined(HAVE_GNUTLS)
/* Temporarily redefine 'id' in case the headers use the objc reserved word.
 */
#define	id	GNUTLSID
/* gcrypt uses __attribute__((deprecated)) to mark structure members that are
 * private.  This causes compiler warnings just from using the header.  Turn
 * them off...
 */
#define	_GCRYPT_IN_LIBGCRYPT
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gcrypt.h>
#undef	id

/* This class is used to ensure that the GNUTLS system is initialised
 * and thread-safe.
 */
@interface      GSTLSObject : NSObject
@end

/* This class provides the current autogenerated Diffie Hellman parameters
 * for server negotiation and/or parameters laoded from file.
 */
@interface      GSTLSDHParams : GSTLSObject
{
  NSDate                *when;
  NSString              *path;
  gnutls_dh_params_t    params;
}

/* Returns the most recently generated key ... if there is none this calls
 * +generate to create one.  Once a key has been generated, replacements
 * are periodically generated in a separate thread.
 */
+ (GSTLSDHParams*) current;

/* Generate key ... really slow.
 */
+ (void) generate;

/* Return params loaded from a file.
 */
+ (GSTLSDHParams*) paramsFromFile: (NSString*)f;

- (gnutls_dh_params_t) params;
@end

/* Manage certificate lists (for servers and clients) and also provide
 * DH params.
 */
@interface      GSTLSCertificateList : GSTLSObject
{
  NSDate                *when;
  NSString              *path;
  gnutls_x509_crt_t     *crts;
  unsigned int          count;
}
+ (GSTLSCertificateList*) listFromFile: (NSString*)f;
- (gnutls_x509_crt_t*) certificateList;
- (unsigned int) count;
@end

/* This encapsulates private keys used to unlock certificates
 */
@interface      GSTLSPrivateKey : GSTLSObject
{
  NSDate                *when;
  NSString              *path;
  NSString              *password;
  gnutls_x509_privkey_t key;
}
+ (GSTLSPrivateKey*) keyFromFile: (NSString*)f withPassword: (NSString*)p;
- (gnutls_x509_privkey_t) key;
@end


/* Declare a pointer to a function to be used for I/O
 */
typedef ssize_t (*GSTLSIOR)(gnutls_transport_ptr_t, void *, size_t);
typedef ssize_t (*GSTLSIOW)(gnutls_transport_ptr_t, const void *, size_t);

/* This class encapsulates a session to a remote system.
 * Sessions are created with a direction and an options dictionary,
 * defining how they will operate.  The handle, pushFunc and pullFunc
 * provide the I/O mechanism, and the host specifies the host that the
 * session is connected to.
 */
@interface      GSTLSSession : GSTLSObject
{
  NSDictionary                          *opts;
  NSHost                                *host;
  GSTLSPrivateKey                       *key;
  GSTLSCertificateList                  *list;
  GSTLSDHParams                         *dhParams;
  gnutls_certificate_credentials_t      certcred;
  BOOL                                  outgoing;
  BOOL                                  active;
  BOOL                                  handshake;
  BOOL                                  setup;
@public
  gnutls_session_t                      session;
}
+ (GSTLSSession*) sessionWithOptions: (NSDictionary*)options
                           direction: (BOOL)isOutgoing
                           transport: (void*)handle
                                push: (GSTLSIOW)pushFunc
                                pull: (GSTLSIOR)pullFunc
                                host: (NSHost*)remote;

- (id) initWithOptions: (NSDictionary*)options
             direction: (BOOL)isOutgoing
             transport: (void*)handle
                  push: (GSTLSIOW)pushFunc
                  pull: (GSTLSIOR)pullFunc
                  host: (NSHost*)remote;

/* Return YES if the session is active (handshake has succeeded and the
 * session has not been disconnected), NO otherwise.
 */
- (BOOL) active;

/* Disconnects and closes down the session.
 */
- (void) disconnect;

/* Try to complete a handshake ... return YES if complete, NO if we need
 * to try again (would have to wait for the remote end).<br />
 */
- (BOOL) handshake;

/* Read data from the session.
 */
- (NSInteger) read: (void*)buf length: (NSUInteger)len;

/** Get a report of the SSL/TLS status of the current session.
 */
- (NSString*) sessionInfo;

/* Write data to the session.
 */
- (NSInteger) write: (const void*)buf length: (NSUInteger)len;

/* For internal use to verify the remmote system's vertificate.
 * Returns 0 on success, negative on failure.
 */
- (int) verify;
@end

#endif

