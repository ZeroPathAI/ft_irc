/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbari <mbari@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/19 00:10:55 by mbari             #+#    #+#             */
/*   Updated: 2022/04/30 20:01:18 by mbari            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#define	USERALREADYJOINED 0
#define USERISJOINED 1
// #define USERISBANNED 2
#define BANNEDFROMCHAN 3
#define TOOMANYCHANNELS 4
#define BADCHANNELKEY 5
#define CHANNELISFULL 6
#define NOSUCHCHANNEL 7


#include "Server.hpp"

class Channel
{
	private:
		char							_prefix;			//
		int								_creatorFd;
		int								_onlineUsers;
		std::string						_name;
		std::string						_key;
		std::string						_topic;
		std::map<int, Client *>			_members;
		std::map<int, Client *>			_operators;			// The first operator is also the one who created the channel need to add "!" to his name
		std::map<int, Client *>			_voice;
		std::map<std::string, Client *> _banned;

	private:
		Channel();

	public:
		Channel( std::string channelName, Client *Creater );
		Channel( std::string channelName, std::string channelKey, Client *Creater );
		Channel( const Channel& x );
		Channel &operator=( const Channel& rhs );
		~Channel();

	public: /*             Getters                         */
		char							const &getPrefix()		const;
		int								const &getOnlineUsers()	const;
		std::string						const &getName() 		const;
		std::string						const &getKey()			const;
		std::string						const &getTopic()		const;
		std::map<int, Client *>			const &getMembers()		const;
		std::map<int, Client *>			const &getOperators()	const;
		std::map<int, Client *>			const &getVoice()		const;
		std::map<std::string, Client *>	const &getBanned()		const;

		Client*		getCreator() const;


	public:
		int		addMember( Client *member );
		void	addOperator( Client *member );
		void	addVoice( Client *member );
		void	addBanned( Client *member );


	public: /*             Setters                         */
		void	setPrefix(char prefix);
		void	setOnlineUsers(int online);
		void	setName(std::string name);
		void	setKey(std::string key);
		void	setTopic(std::string topic);
};

#endif


