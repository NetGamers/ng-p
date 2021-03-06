/**
 * iClient.h
 * Copyright (C) 2002 Daniel Karrels <dan@karrels.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __ICLIENT_H
#define __ICLIENT_H

#include	<string>
#include	<list>
#include	<iostream>
#include	<map>

#include	<ctime>

#include	"Channel.h"
#include	"iServer.h"
#include	"ELog.h"

namespace gnuworld
{

using std::string ;
using std::list ;
using std::map ;

class xClient ;
class Network ;

/**
 * iClient objects represent network users.  This class provides
 * basic functionality and state expected for a network client.
 * Note that iClient is NOT used for services clients, see
 * class xClient for that.
 */
class iClient
{

protected:

	/**
	 * The type used to hold channel patronage information
	 * for each particular user.
	 */
	typedef list< Channel* > channelListType ;

public:

	/**
	 * Define a type to be used for storing the
	 * iClient's modes.
	 */
	typedef unsigned int modeType ;

	/// MODE_OPER is true if the iClient is an IRC operator.
	static const modeType	MODE_OPER ;

	/// MODE_WALLOPS is true if the iClient is receiving wallops.
	static const modeType	MODE_WALLOPS ;

	/// MODE_INVISIBLE is true if the iClient is user mode invisible.
	static const modeType	MODE_INVISIBLE ;

	/// MODE_DEAF is true if the iClient is not receiving messages.
	static const modeType	MODE_DEAF ;

	/// MODE_SERVICES is true if the iClient is a service agent.
	static const modeType	MODE_SERVICES ;

	/// MODE_REGISTERED is true if the iClient has an account set.
	static const modeType	MODE_REGISTERED ;

	/// MODE_HIDDEN_HOST is true if the iClient has HIDDEN_HOST (+x) set.
	static const modeType	MODE_HIDDEN_HOST ;

	/// Iterator for channels this user is on.
	typedef channelListType::iterator channelIterator ;

	/// Constant iterator for channels this user is on.
	typedef channelListType::const_iterator const_channelIterator ;

	/// Type used to store the number of channels for this iClient
	typedef channelListType::size_type channels_sizeType ;

	/**
	 * Construct a new iClient given a large list of
	 * parameters for the client's state.
	 */
	iClient( const unsigned int& _uplink,
		const string& _yyxxx,
		const string& _nickName,
		const string& _userName,
		const string& _hostBase64,
		const string& _insecureHost,
		const string& _realInsecureHost,
		const string& _mode,
		const string& _account,
		const string& _description,
		const time_t& _connectTime ) ;

	/**
	 * Destruct the iClient.
	 * This will call xClient::deleteCustomData() for each
	 * xClient which is storing a data element in this iClient.
	 */
	virtual ~iClient() ;

	/* Accessor methods */

	/**
	 * Retrieve the iClient's nick name.
	 */
	inline const string& getNickName() const
		{ return nickName ; }

	/**
	 * Retrieve the iClient's user name.
	 */
	inline const string& getUserName() const
		{ return userName ; }

	/**
	 * Retrieve the iClient's host name, NOT IP.
	 */
	inline const string& getInsecureHost() const
		{ return insecureHost ;}

	/**
	 * Retrieve the iClient's 'real' host name.
	 */
	inline const string& getRealInsecureHost() const
		{ return realInsecureHost ;}

	/**
	 * Retrieve a string of the form: nick!user@host for this user.
	 */
	inline const string getNickUserHost() const
		{ return (nickName + '!' + userName + '@' + insecureHost) ; }

	/**
	 * Retrieve a string of the form: nick!user@real-host for this user.
	 */
	inline const string getRealNickUserHost() const
		{ return (nickName + '!' + userName + '@' + realInsecureHost) ; }
	
	/**
	 * Retrieve a string of the form: nick!user@fake-host for this user.
	 */
	inline const string getHiddenNickUserHost() const
		{ return (nickName + '!' + userName + '@' + account + HIDDEN_HOST); }

	/**
	 * Retrieve client's 'real-name' field.
	 */
	inline const string& getDescription() const
		{ return description ; }

	/**
	 * Overwrite this clients "Real Host" with a "Hidden Host".
	 */
	inline void setHiddenHost()
		{ insecureHost = account + string(HIDDEN_HOST); }

	/**
	 * Set/Retrieve client's 'account' field.
	 */
	inline const string& getAccount() const
		{ return account ; }

	/**
	 * I really have no idea what this is...someone wanna pass me
	 * a bone here people?
	 */
	inline void setAccount( const string& _account )
		{
		account = _account ;
		setMode(MODE_REGISTERED);
		if (isModeR() && isModeX()) setHiddenHost();
		}

	/**
	 * Retrieve the iClient's connection time.
	 */
	inline const time_t& getConnectTime() const
		{ return connectTime ; }

	/**
	 * Retrieve the iClient's IP.  A network byte ordered
	 * one-word mask is returned.
	 * TODO: Make sure that a word is 32 bits, adjusting where
	 * necessary for > 32bit architectures.
	 */
	inline const unsigned int& getIP() const
		{ return IP ; }

	/**
	 * Obtain const iterator to beginning of this user's channel
	 * membership structure.
	 */
	inline const_channelIterator channels_begin() const
		{ return channelList.begin() ; }

	/**
	 * Obtain const iterator to end of this user's channel
	 * membership structure.
	 */
	inline const_channelIterator channels_end() const
		{ return channelList.end() ; }

	/**
	 * Obtain mutable iterator to beginning of this user's channel
	 * membership structure.
	 */
	inline channelIterator channels_begin()
		{ return channelList.begin() ; }

	/**
	 * Obtain mutable iterator to end of this user's channel
	 * membership structure.
	 */
	inline channelIterator channels_end()
		{ return channelList.end() ; }

	/**
	 * Return the number of channels which this user is on.
	 */
	inline channels_sizeType channels_size() const
		{ return channelList.size() ; }

	/* Mutator methods */

	/**
	 * Add a channel to this user's channel patronage structure.
	 */
	bool addChannel( Channel* newChannel ) ;

	/**
	 * Remove a channel from this user's channel patronage structure.
	 */
	bool removeChannel( Channel* theChannel ) ;

	/**
	 * Clear this client's list of channels.  No heap space needs
	 * to be deallocated here.
	 */
	inline void clearChannels()
		{ channelList.clear() ; }

	/**
	 * Change this iClient's nick name.
	 * During an iClient's lifetime on the network, the only
	 * thing that may change is its nickname.
	 */
	inline void setNickName( const string& newNick )
		{ nickName = newNick ; }

	// The following methods are used to access and mutate
	// the client's modes.

	/**
	 * Return true if this client has the given mode set, false
	 * otherwise.
	 */
	inline bool getMode( const modeType& theMode ) const
		{ return (theMode == (mode & theMode)) ; }

	/**
	 * Return true if this client has the +i mode set, false otherwise.
	 */
	inline bool isModeI() const
		{ return getMode( MODE_INVISIBLE ) ; }

	/**
	 * Return true if this client has the +w mode set, false otherwise.
	 */
	inline bool isModeW() const
		{ return getMode( MODE_WALLOPS ) ; }

	/**
	 * Return true if this client has the +k mode set, false otherwise.
	 */
	inline bool isModeK() const
		{ return getMode( MODE_SERVICES ) ; }

	/**
	 * Return true if this client has the +o mode set, false otherwise.
	 */
	inline bool isModeO() const
		{ return getMode( MODE_OPER ) ; }

	/**
	 * Return true if this client has the +d mode set, false otherwise.
	 */
	inline bool isModeD() const
		{ return getMode( MODE_DEAF ) ; }

	/**
	 * Return true if this client has the +r mode set, false otherwise.
	 */
	inline bool isModeR() const
		{ return getMode( MODE_REGISTERED ) ; }

	/**
	 * Return true if this client has the +x mode set, false otherwise.
	 */
	inline bool isModeX() const
		{ return getMode( MODE_HIDDEN_HOST ) ; }

	/**
	 * Return true if this iClient is an oper, false otherwise.
	 */
	inline bool isOper() const
		{ return getMode( MODE_OPER ) ; }

	/**
	 * Return this iClient's current user modes.
	 */
	inline const modeType& getModes() const
		{ return mode ; }

	/**
	 * Set a user mode for this iClient.
	 */
	inline void setMode( const modeType& newMode )
		{ mode |= newMode ; }

	/**
	 * Set mode +i for this user.
	 */
	inline void setModeI()
		{ setMode( MODE_INVISIBLE ) ; }

	/**
	 * Set mode +w for this user.
	 */
	inline void setModeW()
		{ setMode( MODE_WALLOPS ) ; }

	/**
	 * Set mode +k for this user.
	 */
	inline void setModeK()
		{ setMode( MODE_SERVICES ) ; }

	/**
	 * Set mode +d for this user.
	 */
	inline void setModeD()
		{ setMode( MODE_DEAF ) ; }

	/**
	 * Set mode +o for this user.
	 */
	inline void setModeO()
		{ setMode( MODE_OPER ) ; }

	/**
	 * Set mode +x for this user.
	 */
	inline void setModeX()
		{
		setMode( MODE_HIDDEN_HOST ) ;
		if (isModeR() && isModeX()) setHiddenHost();
		}

	/**
	 * Remove a user mode for this iClient.
	 */
	inline void removeMode( const modeType& theMode )
		{ mode &= ~theMode ; }

	/**
	 * Remove user mode 'i'.
	 */
	inline void removeModeI()
		{ removeMode( MODE_INVISIBLE ) ; }

	/**
	 * Remove user mode 'w'.
	 */
	inline void removeModeW()
		{ removeMode( MODE_WALLOPS ) ; }

	/**
	 * Remove user mode 'k'.
	 */
	inline void removeModeK()
		{ removeMode( MODE_SERVICES ) ; }

	/**
	 * Remove user mode 'd'.
	 */
	inline void removeModeD()
		{ removeMode( MODE_DEAF ) ; }

	/**
	 * Remove user mode 'o'.
	 */
	inline void removeModeO()
		{ removeMode( MODE_OPER ) ; }

	/**
	 * Remove user mode 'x'.
	 */
	inline void removeModeX()
		{ removeMode( MODE_HIDDEN_HOST ) ; }

	/**
	 * Return a string representation of this iClient's user
	 * modes.
	 */
	inline const string getCharModes() const ;

	/**
	 * Return this client's uplink server's integer numeric.
	 */
	inline const unsigned int& getIntYY() const
		{ return intYY ; }

	/**
	 * Return this client's integer per-server numeric.
	 */
	inline const unsigned int& getIntXXX() const
		{ return intXXX ; }

	/**
	 * Return this clients integer network numeric.
	 */
	inline const unsigned int& getIntYYXXX() const
		{ return intYYXXX ; }

	/**
	 * Return this client's uplink server's character numeric.
	 */
	inline const char* getCharYY() const
		{ return charYY ; }

	/**
	 * Return this client's character per-server numeric.
	 */
	inline const char* getCharXXX() const
		{ return charXXX ; }

	/**
	 * Return this client's full character numeric.
	 */
	inline const string getCharYYXXX() const
		{ return( string( charYY ) + charXXX ) ; }

	/**
	 * Add an xClient's personal data representation to this
	 * iClient.  Only one element per xClient is permitted.
	 * This method will return false if the internal allocation
	 * fails, or if the xClient in question is already using
	 * a data element in this iClient, or if the addition to the
	 * internal data structure fails.
	 * Add a NULL value to this iClient is permitted.
	 */
	bool		setCustomData( xClient*, void* ) ;

	/**
	 * Retrieve the custom data element for the given xClient.
	 * If none is found, NULL is returned.
	 */
	void*		getCustomData( xClient* ) const ;

	/**
	 * Retrieve, remove, and return the custom data element
	 * for the given xClient.  If none is found, NULL is
	 * returned.
	 */
	void*		removeCustomData( xClient* ) ;

	/**
	 * operator<< is overloaded for debugging purposes,
	 * this makes it extremely easy to output this client's
	 * information to an output stream.
	 */
	inline friend ELog& operator<<( ELog& out,
		const iClient& theClient )
		{
		out     << theClient.nickName << '!'
			<< theClient.userName << '@'
			<< theClient.insecureHost
			<< " Numeric: " << theClient.getCharYYXXX()
			<< ", int YY/XXX/YYXXX: "
			<< theClient.getIntYY() << '/'
			<< theClient.getIntXXX() << '/'
			<< theClient.getIntYYXXX()
			<< std::endl ;
		return out ;
		}

protected:

	/**
	 * Disable copy constructor.  This method is declared but
	 * NOT defined.
	 */
	iClient( const iClient& ) ;

	/**
	 * Disable default assignment.  This method is declared but
	 * NOT defined.
	 */
	iClient operator=( const iClient& ) ;

	/**
	 * Parse a string of modes and set this iClient's
	 * modes appropriately.
	 * This is called by the constructor only.
	 */
	void		setModes( const string& ) ;

	// The below variables are ordered to provide
	// efficient instantiation.  Do not alter order.

	/**
	 * This is the integer representation of this
	 * client's uplink server.
	 */
	unsigned int	intYY ;

	/**
	 * This client's nick name.
	 */
	string		nickName ;

	/** This client's user name.
	 */
	string		userName ;

	/**
	 * This client's 32 bit IP, stored in
	 * network (bigendian) byte order.
	 */
	unsigned int	IP ;

	/**
	 * This client's hostname as it appears to network users.
	 * (Possibly a hidden-hostname is the user is +x)
	 */
	string		insecureHost ;

	/**
	 * This client's actual network hostname, unhidden and
	 * exposed.
	 */
	string		realInsecureHost ;

	/**
	 * This client's 'real-name' field data.
	 */
	string		description ;

	/**
	 * The time at which this iClient connected to the network.
	 */
	time_t		connectTime ;

	/**
	 * This client's current user modes.
	 */
	modeType	mode ;

	/**
	 * This client's "Account".
	 */
	string		account ;

	/**
	 * This client's integer per-server numeric.
	 */
	unsigned int	intXXX ;

	/**
	 * This client's uplink server's character numeric.
	 */
	char		charYY[ 3 ] ;

	/**
	 * This client's per-server character numeric.
	 */
	char		charXXX[ 4 ] ;

	/**
	 * This client's integer network numeric.
	 */
	unsigned int	intYYXXX ;

	/**
	 * The structure used to store which channels this user is in.
	 */
	channelListType	channelList ;

	/**
	 * This is the type used to represent the custom data map.
	 */
	typedef map< xClient*, void* >	customDataMapType ;

	/**
	 * This structure is used to store generic data for
	 * individual xClient's.  Only one element per xClient is
	 * permitted to be stored in this structure.  A pointer is
	 * used here to reduce memory footprint for those iClient's
	 * whose customDataMap's aren't used.
	 */
	customDataMapType		*customDataMap ;

} ;

} // namespace gnuworld

#endif // __ICLIENT_H
