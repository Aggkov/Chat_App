#include "includes/UserDatabase.h"
#include "includes/network_headers.h"
#include "../Utils/string_utils.h"
#include "includes/User.h"
#include "includes/ThreadPool.h"


class Server{
public:
	Server() = default;
	void start(const std::string& host, int port);
	int create_server_socket_bind_listen(const std::string& host, int port);
	void handle_new_connection(const SOCKET server_socket, fd_set& masterSet, SOCKET& maxSock, std::unordered_map<SOCKET, User>& m_users);
	void handle_client_message(SOCKET client_socket, fd_set& master_Set,std::unordered_map<SOCKET, User>& users);
	void broadcast_message(std::string message, SOCKET sender_socket, std::unordered_map<SOCKET, User>& users);
	void send_error(SOCKET client_socket, std::string& error);
	std::string get_client_address(User& user);
	//int select_read(SOCKET server_socket);
private:
	SOCKET m_server_sock = INVALID_SOCKET;
	std::mutex m_mtx;
};