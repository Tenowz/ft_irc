#include "channel.hpp"

Channel::Channel()
{
}

Channel::Channel(Client *admin, std::string name, std::string key)
{
	this->name = name;
	this->admin = admin;
	this->key = key;
	this->i = false;
	this->t = false;
	this->k = true;
	this->l = false;
	this->limit = 0;

	std::cout << "new channel " << this->name << " created by " << admin->get_nickname() << std::endl;
}

Channel::~Channel()
{
}

Client  *Channel::find_client(std::string client_name)
{
	for (size_t i = 0; i < this->client_list.size(); i++)
	{
		if (this->client_list[i]->get_nickname() == client_name)
			return (this->client_list[i]);
	}
	return NULL;
}

Client	*Channel::findInvited(int const socket) {

	for (size_t i = 0; i < this->invit_list.size(); i++) {
		
		if (this->invit_list[i]->get_socket() == socket)
			return (this->invit_list[i]);
	}
	return NULL;
}

bool    Channel::is_op(Client *client)
{
	if (client == this->admin)
		return true;
	for (size_t i = 0; i < this->op_list.size(); i++)
	{
		if (this->op_list[i] == client)
			return true;
	}
	return false;
}

void Channel::add_client(Client *client)
{
	for (size_t i = 0; i < this->client_list.size(); i++)
	{
		if (this->client_list[i] == client)
			return ;
	}
	this->client_list.push_back(client);
}

void Channel::remove_client(Client *client)
{
	std::vector<Client *>::iterator it;
	for (it = client_list.begin(); it != client_list.end(); it++)
	{
		if (*it == client) {
			client_list.erase(it);
			std::cout << client->get_nickname() << " removed from " << this->name << std::endl;
			return ;
		}
	}
}

void	Channel::setLimit(std::string limit, int socket) {

	for (size_t i = 0; i < limit.size(); i++) {
		
		if (limit[i] < '0' || limit[i] > '9') {

			std::string	msg = "Wrong arguments !\r\n";
			send(socket, msg.c_str(), msg.size(), 0);
			return ;
		}
	}
	
}