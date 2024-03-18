#include "client.hpp"

Client::Client()
{
	socket = 0;
	username = "";
	nickname = "";
}

Client::Client(int socket_fd, std::string username, std::string nickname) : socket(socket_fd), username(username), nickname(nickname) {}

Client::~Client()
{
	std::cout << "Client destructor called" << std::endl;
}

Client  *findUsername(std::vector<Client *> client_list, std::string username)
{
	for (size_t i = 0; i < client_list.size(); i++)
	{
		if (client_list[i]->get_username() == username)
			return (client_list[i]);
	}
	return NULL;
}

Client  *findNickname(std::vector<Client *> client_list, std::string nickname)
{
	for (size_t i = 0; i < client_list.size(); i++)
	{
		if (client_list[i]->get_nickname() == nickname)
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
	std::string cmd = input.substr(0, input.find_first_of(" \t"));
	std::vector<std::string> params;
	
	size_t pos = 0;
	std::string token;
	while ((pos = input.find_first_of(" \t")) != std::string::npos) {
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
		return (this->nick(params[0], client_list));
	else if (cmd == "USER")
		return (this->user(params[0], client_list));
	else if (cmd == "PRIVMSG")
		return (this->privmsg(params, channel_list, client_list));
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

int Client::nick(std::string params, std::vector<Client *> &client_list)
{
	std::string	msg;

	if (params.empty() || !params[0] || params.find_first_of(" \t\n\v\f\r") != std::string::npos || params == "NICK" || findNickname(client_list, params)) {

		msg = "Invalid nickname !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	int i = std::min(params.find(' '), params.find('\r'));
	this->nickname = params.substr(0, i);
	std::cout << this->nickname << " nickname's now set to " << params << std::endl;
	msg = params + " is your new nickname\r\n";
	send(this->socket, msg.c_str(), msg.size(), 0);
	return 0;
}

int Client::user(std::string params, std::vector<Client *> &client_list)
{
	std::string	msg;

	if (params.empty() || !params[0] || params.find_first_of(" \t\n\v\f\r") != std::string::npos || params == "USER" || findUsername(client_list, params)) {

		msg = "Invalid username !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	int i = std::min(params.find(' '), params.find('\r'));
	this->username = params.substr(0, i);
	std::cout << this->username << " username's now set to " << params << std::endl;
	msg = params + " is your new username\r\n";
	send(this->socket, msg.c_str(), msg.size(), 0);
	return 0;
}

int Client::join(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string msg;
	size_t		i = 0;
	size_t		j = -1;
	size_t		key = 0;

	for (std::vector<std::string>::iterator it = params.begin(); it != params.end() && ((*it)[0] == '#' || (*it)[0] == '&'); ++it)
		i++;
	while (++j < i) {

		if (channel_list.count(params[j]) == 0) {

			if (i + key >= params.size() || params[i + key].empty() || !params[i + key][0] || isEven(params)) {

				std::cout << "Channel creation attempt failed" << std::endl;
				msg = "Invalid arguments !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				return 0;
			}
			channel_list.insert(std::pair<std::string, Channel*>(params[j], new Channel(this, params[j], params[i + key++])));
			msg = "You have created the channel " + params[j] + " and your password is " + channel_list[params[j]]->get_key() + "\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			channel_list[params[j]]->add_client(this, channel_list[params[j]]->get_client_list());
		}
		else if (channel_list.count(params[j])) {

			if (find_client_socket(channel_list[params[j]]->get_client_list(), this)) {

				msg = "You have already joined the channel !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				continue;
			}
			if (channel_list[params[j]]->getKeyState() == true && params[i + key] != channel_list[params[j]]->get_key()) {

				msg = "Wrong password !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				continue;
			}
			else
				key++;
			if (channel_list[params[j]]->getLimitState() == true && channel_list[params[j]]->getLimit() && channel_list[params[0]]->get_client_list().size() >= channel_list[params[0]]->getLimit()) {

				msg = "The channel is already full !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				continue;
			}
			if (channel_list[params[j]]->getInviteState() == true && !channel_list[params[j]]->findInvited(this->socket)) {

				msg = "You are not invited !\r\n";
				send(this->socket, msg.c_str(), msg.size(), 0);
				continue;
			}
			std::cout << this->nickname << " joined the channel " << params[j] << std::endl;
			msg = "You have successfully joined the channel !\r\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			msg = this->nickname + " joined the channel " + params[j] + "\r\n";
			sendtochannel(channel_list[params[j]], msg);
			channel_list[params[j]]->add_client(this, channel_list[params[j]]->get_client_list());
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
	channel_list[channel_name]->remove_client(this, channel_list[channel_name]->get_client_list());
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
	channel_list[channel_name]->remove_client(kicked, channel_list[channel_name]->get_client_list());

	return 0;
}

int Client::mode(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list)
{
	std::string	msg;
    std::string channel_name = params[0];
    std::string options = params[1];
    size_t i = 0;
    size_t j = 2;
    bool state;

	if (params.size() < 2 || channel_list.find(params[0]) == channel_list.end()) {

		msg = "Invalid arguments !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	else if (channel_list[params[0]]->is_op(this) == false) {

		msg = "You're not allowed to modificate !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	else if (options[i] == '+')
        state = true;
    else if (options[i] == '-')
        state = false;
    else
        return 0;
    while (i < options.size())
    {
        if (options[i] == 'i')
            channel_list[channel_name]->invite_only(state);
		else if (options[i] == 't')
			channel_list[channel_name]->topicOpOnly(state);
		else if (options[i] == 'k')
			channel_list[channel_name]->keyOnly(state);
        else if (options[i] == 'o')
        {
            if (state) {

				if (params.size() < 3 || !channel_list[channel_name]->find_client(params[j])) {

					msg = "Client not found !\r\n";
					send(this->socket, msg.c_str(), msg.size(), 0);
					return 0;
				}
				channel_list[channel_name]->add_client(channel_list[channel_name]->find_client(params[j]), channel_list[channel_name]->getOpList());
			}
            else {

				if (params.size() < 3 || !channel_list[channel_name]->find_client(params[j])) {

					msg = "Client not found !\r\n";
					send(this->socket, msg.c_str(), msg.size(), 0);
					return 0;
				}
				channel_list[channel_name]->remove_client(channel_list[channel_name]->find_client(params[j]), channel_list[channel_name]->getOpList());
			}
            j++;
        }
        else if (options[i] == 'l')
        {
            channel_list[channel_name]->limitOnly(state);
            if (state) {

				if (params.size() < 3 || atoi(params[j].c_str()) < 1) {

					msg = "Invalid limit !\r\n";
					send(this->socket, msg.c_str(), msg.size(), 0);
					return 0;
				}
				channel_list[channel_name]->setLimit(params[j], this->socket);
			}
            j++;
        }
        i++;
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
		channel_list[*it]->remove_client(this, channel_list[*it]->get_client_list());

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
	Client *invited = findNickname(client_list, invited_name);
	if (!invited) {

		message = "INVITE ERROR : user not found in channel\r\n";
		send(this->socket, message.c_str(), message.size(), 0);
		return 0;
	}
	channel_list[channel_name]->addInvite(invited);
	message = this->nickname + " INVITE " + invited_name + ' ' + channel_name + "\r\n";
	send(invited->get_socket(), message.c_str(), message.size(), 0);
	sendtochannel(channel_list[channel_name], message);
	return 0;
}

int Client::privmsg(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list)
{
	std::string	msg;
	std::string name = params[0];
	std::string message = params[1];
	
	if (name.empty() || message.empty() || params.size() > 2 || message[0] != ':') {

		msg = "Invalid arguments !\r\n";
		send(this->socket, msg.c_str(), msg.size(), 0);
		return 0;
	}
	if (name[0] == '#' || name[0] == '&') {

		if (channel_list.count(name) == 0) {

			msg = "Channel not found !\r\n";
			send(this->socket, message.c_str(), message.size(), 0);
			return 0;
		}
		message = ":" + this->nickname + " PRIVMSG " + name + message + "\r\n";
		sendtochannel(channel_list[name], message);
	}
	if (name[0] != '#' && name[0] != '&') {
		
		if (!findNickname(client_list, name)) {

			msg = "Client not found !\r\n";
			send(this->socket, msg.c_str(), msg.size(), 0);
			return 0;
		}
		message = ":" + this->nickname + " PRIVMSG " + name + " :" + message + "\r\n";
		send(findNickname(client_list, name)->socket, message.c_str(), message.size(), 0);
	}
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

bool	isEven(std::vector<std::string> params) {

	for (size_t i = 0; i < params.size() && (params[i][0] == '#' || params[i][0] == '&'); i++) {

		for (size_t j = 0; j < params.size() && (params[j][0] == '#' || params[j][0] == '&'); j++) {

			if (i == j)
				j++;
			if (params[i] == params[j] && (params[i][0] == '#' || params[i][0] == '&') && (params[j][0] == '#' || params[j][0] == '&'))
            	return true;
		}
    }
    return false;
}