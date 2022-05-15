/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   JoinCommand.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbari <mbari@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/12 17:21:00 by mbari             #+#    #+#             */
/*   Updated: 2022/05/15 13:14:05 by mbari            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/Server.hpp"


std::string	Server::_joinChannel( Request request, int i )
{
	int j = 1;
	if (!this->_clients[i]->getRegistered())
		return (_printMessage("451", this->_clients[i]->getNickName(), ":You have not registered"));
	if (request.args.size() == 0)
		return (_printMessage("461", this->_clients[i]->getNickName(), " :Not enough parameters"));
	if (request.args[0] == "0")
		return(this->_clients[i]->leaveAllChannels());
	std::vector<std::string> parsChannels(_commaSeparator(request.args[0]));
		std::vector<std::string> parsKeys;
	if (request.args.size() == 2)
		parsKeys = _commaSeparator(request.args[1]);
	std::vector<std::string>::iterator itChannels = parsChannels.begin();
	std::vector<std::string>::iterator itKeys = parsKeys.begin();
	while (itChannels != parsChannels.end() && j == 1)
	{
		if ( itKeys != parsKeys.end())
			j = _createPrvChannel(*itChannels, *itKeys, i);
		else
			j = _createChannel(*itChannels, i);
		if (j == BADCHANMASK)
			return (_printMessage("476", this->_clients[i]->getNickName(), *itChannels + " :Bad Channel Mask"));
		if (j == BANNEDFROMCHAN)
			return (_printMessage("474", this->_clients[i]->getNickName(), *itChannels + " :Cannot join channel (+b)"));
		if (j == TOOMANYCHANNELS )
			return (_printMessage("405", this->_clients[i]->getNickName(), *itChannels + " :You have joined too many channels"));
		if (j == BADCHANNELKEY )
			return (_printMessage("475", this->_clients[i]->getNickName(), *itChannels + " :Cannot join channel (+k)"));
		if (j == CHANNELISFULL )
			return (_printMessage("471", this->_clients[i]->getNickName(), *itChannels + " :Cannot join channel (+l)"));
		if (j == NOSUCHCHANNEL)
			return (_printMessage("403", this->_clients[i]->getNickName(), *itChannels + " :No such channel"));
		if (j == USERISJOINED)
		{
			_sendall(i, this->_clients[i]->getUserPerfix() + "JOIN " + *itChannels + "\n");
			_sendall(i, _printReply() )
		}
		if (itKeys != parsKeys.end())
			itKeys++;
		itChannels++;
	};
	--itChannels;
	return (_printMessage("332", this->_clients[i]->getNickName(), *itChannels + " :" + this->_allChannels.find(*itChannels)->second->getTopic()));
};

int	Server::_createChannel( std::string ChannelName, int CreatorFd )
{
	std::map<std::string, Channel *>::iterator it = this->_allChannels.find(ChannelName);
	if (it == this->_allChannels.end())
	{
		if (ChannelName[0] != '&' && ChannelName[0] != '#' && ChannelName[0] != '+' && ChannelName[0] != '!')
			return (BADCHANMASK);
		Channel *channel = new Channel(ChannelName, this->_clients[CreatorFd]);
		this->_allChannels.insert(std::pair<std::string, Channel *>(ChannelName, channel));
		this->_clients[CreatorFd]->joinChannel( ChannelName, channel );
	}
	else
	{
		if (it->second->getKey().empty())
		{
			int i = 0;
			if (this->_clients[CreatorFd]->getisOperator() == true)
				i = it->second->addOperator(this->_clients[CreatorFd]);
			else
				i = it->second->addMember(this->_clients[CreatorFd]);
			if (i == USERISJOINED)
				this->_clients[CreatorFd]->joinChannel( it->first, it->second );
			else if (i == USERALREADYJOINED)
				return (USERALREADYJOINED);
			else if (i == BANNEDFROMCHAN)
				return (BANNEDFROMCHAN);
			std::string reply = "JOIN " + ChannelName + "\n";
			_sendToAllUsers(it->second, CreatorFd, reply);
			return (USERISJOINED);
		}
	}
	return (USERISJOINED);
};

int	Server::_createPrvChannel( std::string ChannelName, std::string ChannelKey, int CreatorFd)
{
	std::map<std::string, Channel *>::iterator it = this->_allChannels.find(ChannelName);
	if (it == this->_allChannels.end())
	{
		if (ChannelName[0] != '&' && ChannelName[0] != '#' && ChannelName[0] != '+' && ChannelName[0] != '!')
			return (BADCHANMASK);
		Channel *channel = new Channel(ChannelName, ChannelKey, this->_clients[CreatorFd]);
		this->_allChannels.insert(std::pair<std::string, Channel *>(ChannelName, channel));
		this->_clients[CreatorFd]->joinChannel(ChannelName, channel);
	}
	else
	{
		if (it->second->getKey() == ChannelKey)
		{
			int i = 0;
			if (this->_clients[CreatorFd]->getisOperator() == true)
				i = it->second->addOperator(this->_clients[CreatorFd]);
			else
				i = it->second->addMember(this->_clients[CreatorFd]);
			if (i == USERISJOINED)
				this->_clients[CreatorFd]->joinChannel( it->first, it->second );
			else if (i == USERALREADYJOINED)
				return (USERALREADYJOINED);
			else if (i == BANNEDFROMCHAN)
				return (BANNEDFROMCHAN);
			std::string reply = "JOIN " + ChannelName + "\n";
			_sendToAllUsers(it->second, CreatorFd, reply);
			return (USERISJOINED);
		}
		else
			return (BADCHANNELKEY);
	}
	return (USERISJOINED);
};

std::vector<std::string> Server::_commaSeparator(std::string arg)
{
	std::vector<std::string> ret;
	int pos = 0;
	std::cout << "start parsing" << std::endl;
	while ((pos = arg.find(",")) != std::string::npos)
	{
		ret.push_back(arg.substr(0, pos));
		std::cout << ret.back() << std::endl;
		arg.erase(0, pos + 1);
	}
	ret.push_back(arg.substr(0, pos));
	std::cout << ret.back() << std::endl;
	return (ret);
};
