#include "client.hpp"

Client::Client()
{
	socket = 0;
	username = "";
	nickname = "";
}

Client::Client(int socket_fd)
{
	socket = socket_fd;
	username = "";
	nickname = "";
}

Client::~Client()
{
	std::cout << "client destructor called" << std::endl;
}

Client  *find_client(std::vector<Client *> client_list, std::string client_name)
{
	for (size_t i = 0; i < client_list.size(); i++)
	{
		if (client_list[i]->get_nickname() == client_name)
			return (client_list[i]);
	}
	return NULL;
}

int	find_client_socket(std::vector<Client *> client_list, Client *client) {

	for (size_t i = 0; i < client_list.size(); i++) {

		if (client_list[i]->get_socket() == client->get_socket())
			return client->get_socket();
	}
	return 0;
}

int Client::parse_cmd(std::string input, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list)
{
	size_t pos = 0;
	std::string token = input;
	while ((pos = input.find("\r\n")) != std::string::npos) {
		token = input.substr(0, pos);
		std::cout << token << std::endl;
		this->exec_cmd(token, channel_list, client_list);
		input.erase(0, pos + 2);
	}
	this->exec_cmd(input, channel_list, client_list);
	return 0;
}

int	Client::exec_cmd(std::string input, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list)
{
	std::string cmd = input.substr(0, input.find(' '));
	std::vector<std::string> params;
	
	size_t pos = 0;
	std::string token;
	while ((pos = input.find(' ')) != std::string::npos) {
		if (input[0] == ':')
			break ;
		token = input.substr(0, pos);
		params.push_back(token);
		input.erase(0, pos + 1);
	}
	params.push_back(input);
	params.erase(params.begin());

	if (cmd == "JOIN")
		return (this->join(params, channel_list));
	else if (cmd == "NICK")
		return (this->nick(params[0]));
	else if (cmd == "USER")
		return (this->user(params[0]));
	else if (cmd == "PRIVMSG")
		return (this->privmsg(params, channel_list));
	else if (cmd == "PART")
		return (this->part(params, channel_list));
	else if (cmd == "QUIT")
		return (this->leave_channels(channel_list));
	else if (cmd == "MODE")
		return (this->mode(params, channel_list));
	else if (cmd == "KICK")
		return (this->kick(params, channel_list));
	else if (cmd == "INVITE")
		return (this->invite(params, channel_list, client_list));
	else if (cmd == "TOPIC")
		return (this->topic(params, channel_list));
	else {
		std::cout << "UNKNOWN CMD" << std::endl;
	}
	return (0);
}

int Client::nick(std::string params)
{
	int i = std::min(params.find(' '), params.find('\r'));
	this->nickname = params.substr(0, i);

	std::cout << "set nickname to " << this->nickname << "|" << std::endl;

	return 0;
}

int Client::user(std::string params)
{
	int i = std::min(params.find(' '), params.find('\r'));
	this->username = params.substr(0, i);

	std::cout << "set username to " << this->username << "|" << std::endl;

	return 0;
}

int Client::join(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string msg;
	size_t		i = 0;
	size_t		j = -1;

	if (channel_list[params[0]]->getKeyState() == true) {

		for (std::vector<std::string>::iterator it = params.begin(); it != params.end() && ((*it)[0] == '#' || (*it)[0] == '&') ; ++it)
			++i;

		if (params.size() % 2 != 0 || i != params.size() / 2) {

			msg = "Wrong arguments\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			return 0;
		}
	} else if (channel_list[params[0]]->getKeyState() == false) {

		for (std::vector<std::string>::iterator it = params. begin(); it != params.end() && ((*it)[0] == '#' || (*it)[0] == '&'); ++it)
			i++;
		
		if (i != params.size()) {

			msg = "Wrong arguments\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			return 0;
		}
	}
	while (++j < i) {

		if (channel_list.count(params[j]) == 0 && channel_list[params[0]]->getKeyState() == true) {

			channel_list.insert(std::pair<std::string, Channel*>(params[j], new Channel(this, params[j], params[params.size() / 2 + j])));
			msg = "You have created the channel " + params[j] + " and your password is " + channel_list[params[j]]->get_key() + "\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			channel_list[params[j]]->add_client(this);
		}
		else if (channel_list.count(params[j])) {

			if (find_client_socket(channel_list[params[j]]->get_client_list(), this)) {

				msg = "You've already joined the channel !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				return 0;
			}
			if (channel_list[params[0]]->getKeyState() == true && params[params.size() / 2 + j] != channel_list[params[j]]->get_key()) {

				msg = "Wrong password !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				return 0;
			}
			if (channel_list[params[0]]->getLimitState() == true && channel_list[params[0]]->getLimit() && channel_list[params[0]]->get_client_list().size() >= channel_list[params[0]]->getLimit()) {

				msg = "The channel is already full !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				return 0;
			}
			if (channel_list[params[0]]->getInviteState() == true && !channel_list[params[0]]->findInvited(this->socket)) {

				msg = "You're not invited !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				return 0;
			}
			std::cout << this->nickname << " joined the channel " << params[j] << std::endl;
			msg = "You've successfully joined the channel !\r\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			msg = this->nickname + " joined the channel " + params[j] + "\r\n";
			sendtochannel(channel_list[params[j]], msg);
			channel_list[params[j]]->add_client(this);
		}
	}
	return 0;
}

int Client::part(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string channel_name = params[0];
	if (channel_list.count(channel_name) == 0) {
		std::cout << "PART ERROR : channel not found" << std::endl;
		return 0;
	}
	channel_list[channel_name]->remove_client(this);
	std::string message = ":" + this->nickname + " PART " + channel_name + "\r\n";
	send(this->socket, message.c_str(), message.size(), 0);
	sendtochannel(channel_list[channel_name], message);
	return (0);
}

int Client::kick(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string channel_name = params[0];
	std::string kicked_name = params[1];

	if (channel_list.count(channel_name) == 0) {
		std::cout << "KICK ERROR : channel not found" << std::endl;
		return 0;
	}

	if (channel_list[channel_name]->is_op(this) == false)
	{
		std::cout << "KICK ERROR : user is not allowed to kick" << std::endl;
		return 0;
	}

	Client  *kicked = channel_list[channel_name]->find_client(kicked_name);
	if (!kicked)
	{
		std::cout << "KICK ERROR : user not found in channel" << std::endl;
		return 0;
	}

	std::string message = ":" + this->nickname + " KICK " + channel_name + ' ' + kicked_name + "\r\n";
	send(this->socket, message.c_str(), message.size(), 0);
	sendtochannel(channel_list[channel_name], message);

	channel_list[channel_name]->remove_client(kicked);

	return 0;
}

int Client::mode(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string	msg;
	std::string	sign;
	std::string	nbArg;

	if (channel_list[params[0]]->is_op(this) == false) {

		msg = "You're not allowed to modificate !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	if (channel_list.find(params[0]) == channel_list.end()) {

		msg = "Channel does not exist !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
	}
	else if (channel_list.find(params[0]) != channel_list.end()) {

		if (params[2][0] == '-' || params[2][0] == '+') {

			if (params[2][0] == '-')
				sign = '-';
			else
				sign = '+';
			for (size_t i = 1; i < params[2].size(); i++) {

				if (params[2][i] == 'i' && params[2][0] == '-')
					channel_list[params[0]]->invite_only(false);
				else if (params[2][i] == 'i' && params[2][0] == '+')
					channel_list[params[0]]->invite_only(true);
				else if (params[2][i] == 't' && params[2][0] == '-')
					channel_list[params[0]]->topicOpOnly(false);
				else if (params[2][i] == 't' && params[2][0] == '+')
					channel_list[params[0]]->topicOpOnly(true);
				else if (params[2][i] == 'k' && params[2][0] == '-')
					channel_list[params[0]]->keyOnly(false);
				else if (params[2][i] == 'k' && params[2][0] == '+')
					channel_list[params[0]]->keyOnly(true);
				else if (params[2][i] == 'o' && params[2][0] == '-')
					nbArg += 'o';
				else if (params[2][i] == 'o' && params[2][0] == '+')
					nbArg += 'o';
				else if (params[2][i] == 'l' && params[2][0] == '-')
					nbArg += 'l';
				else if (params[2][i] == 'l' && params[2][0] == '-')
					nbArg += 'l';
			}
		}
		size_t	o = nbArg.find('o');
		size_t	l = nbArg.find('l');
		if (o == std::string::npos && l == std::string::npos)
			return 0;
		if (o < l) {

			channel_list[params[0]]->getOpList().push_back(channel_list[params[0]]->find_client(params[3]));
			if (l != std::string::npos) {

				(sign[0] == '-') ? channel_list[params[0]]->limitOnly(false) : channel_list[params[0]]->limitOnly(true);
				channel_list[params[0]]->setLimit(params[3], this->socket);
			}
		}
		else if (l < o) {

			(sign[0] == '-') ? channel_list[params[0]]->limitOnly(false) : channel_list[params[0]]->limitOnly(true);
			channel_list[params[0]]->setLimit(params[3], this->socket);
			if (o != std::string::npos)
				channel_list[params[0]]->getOpList().push_back(channel_list[params[0]]->find_client(params[3]));
		}
	}
	return 0;
}

int	Client::topic(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string	msg;

	if (params.size() == 1 && channel_list.find(params[0]) != channel_list.end()) {

		if (channel_list[params[0]]->getTopic().empty())
			msg = "No topic set !\r\n";
		else
			msg = params[0] + "\'s topic " + channel_list[params[0]]->getTopic() + "\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	if (params.size() == 2 && channel_list.find(params[0]) != channel_list.end() && params[1][0] == ':') {

		if (channel_list[params[0]]->getTopicState() == true && channel_list[params[0]]->is_op(this) == false) {

			msg = "You're not allowed to change this channel's topic !\r\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			return 0;
		}
		channel_list[params[0]]->setTopic(params[1]);
		std::cout << "Channel " << channel_list[params[0]]->get_name() << "\'s topic successfully changed" << std::endl;
		msg = channel_list[params[0]]->get_name() + "\'s new topic is " + channel_list[params[0]]->getTopic() + "\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	std::cout << "Changing or viewing " << params[0] << "\'s topic failed" << std::endl;
	msg = "Unable to change or view " + params[0] + "\'s topic\r\n";
	send(this->socket, msg.c_str(), msg.size(), 0);
	return 0;
}

int Client::leave_channels(std::map<std::string, Channel*> &channel_list)
{
	std::set<std::string>::iterator it;
	for (it = this->channels.begin(); it != this->channels.end(); it++)
	{
		channel_list[*it]->remove_client(this);
	}
	return 0;
}

int Client::invite(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list)
{
	std::string message;
	std::string invited_name = params[0];
	std::string channel_name = params[1];

	if (channel_list.count(channel_name) == 0) {

		message = "INVITE ERROR : channel not found\r\n";
		send(this->socket, message.c_str(), message.size(), 0);
		return 0;
	}
	if (channel_list[channel_name]->is_op(this) == false) {

		message = "INVITE ERROR : user is not allowed to invite\r\n";
		send(this->socket, message.c_str(), message.size(), 0);
		return 0;
	}
	Client *invited = find_client(client_list, invited_name);
	if (!invited) {

		message = "INVITE ERROR : user not found in channel\r\n";
		send(this->socket, message.c_str(), message.size(), 0);
		return 0;
	}
	channel_list[channel_name]->getInviteList().push_back(invited);
	message = this->nickname + " INVITE " + invited_name + ' ' + channel_name + "\r\n";
	send(invited->get_socket(), message.c_str(), message.size(), 0);
	sendtochannel(channel_list[channel_name], message);
	return 0;
}

int Client::privmsg(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string	msg;
	std::string channel_name = params[0];
	std::string message = params[1];
	
	if (channel_list.count(channel_name) == 0) {

		msg = "Channel not found !\r\n";
		send(this->socket, message.c_str(), message.size(), 0);
		return 0;
	}
	message = ":" + this->nickname + " PRIVMSG " + channel_name + " :" + message + "\r\n";
	sendtochannel(channel_list[channel_name], message);
	return 0;
}

int Client::sendtochannel(Channel *channel, std::string message)
{
	int send_socket;
	int n = channel->get_client_list().size();
	for (int i = 0; i < n; i++)
	{
		send_socket = channel->get_client_list()[i]->get_socket();
		if (send_socket != this->socket)
		{
			send(send_socket, message.c_str(), message.size(), 0);
			std::cout << "message send to " << send_socket << std::endl;
		}
	}
	return 0;
}
