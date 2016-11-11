#include <iostream>

#include "Session.h"
#include "Game_server.h"

/*
* ���ǿ� ���� �ʱ�ȭ�� �����Ѵ�.
*/
Session::Session(int Session_id, boost::asio::io_service& io_service, GameServer *server)
	: session_id(Session_id), m_socket(io_service), game_server(server)
{
}

/*
* ������ �����ϰ� �ڿ����� ��ȯ�Ѵ�.
*/
Session::~Session()
{
}

/*
* ������ �ʱ�ȭ�Ѵ�.
*/
void Session::Init()
{
	m_nPacketBufferMark = 0;
}

/*
* ���ǿ� ����� Ŭ���̾�Ʈ�� ��Ŷ�� �����Ѵ�.
*/
void Session::Send_packet(char *packet, size_t size)
{

}

/* 
* ������ receive�� �����Ѵ�.
*/
void Session::Receive_packet()
{
	m_socket.async_read_some(
		boost::asio::buffer(m_ReceiveBuffer),
		boost::bind(&Session::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void Session::handle_write(const boost::system::error_code& error,
	size_t bytes_transferred)
{

}

void Session::handle_receive(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cerr << "[ERROR] Connection to the client has been lost." << std::endl;
		}
		else
		{
			std::cerr << "[ERROR] No: " << error.value() << " Message: "
				<< error.message() << std::endl;
		}

		game_server->CloseSession(session_id);  // ������ �߻��Ͽ� ���ǰ� ������ �����մϴ�.
	}
	else
	{
		// receive ���ۿ� ����� ��Ŷ�� packet ���۷� �����մϴ�.
		memcpy(&m_PacketBuffer[m_nPacketBufferMark], m_ReceiveBuffer.data(), bytes_transferred);

		int nPacketData = m_nPacketBufferMark + bytes_transferred;  // ������ �ִ� ������ + ���� �޾ƿ� ��
		int nReadData = 0;
		
		protobuf::io::ArrayInputStream input_array_stream(&m_PacketBuffer, nPacketData);
		protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

		m_nPacketBufferMark = Process_packet(input_coded_stream, 
			game_server->packet_handler); // ������ ��Ŷ ���ۿ� ���� ��Ŷ���� ó���ϰ� ���� ��Ŷ �������� ���� �����մϴ�.
		nReadData = nPacketData - m_nPacketBufferMark;  // ó���� �������� ��

		if (m_nPacketBufferMark > 0) // ���� �����͸� �����մϴ�.
		{
			google::protobuf::uint8 TempBuffer[MAX_RECEIVE_BUF_LEN] = { 0, };
			memcpy(&TempBuffer[0], &m_PacketBuffer[nReadData], m_nPacketBufferMark); // ���������� ��Ŷ�� �о��� ������ ���縦 ��
			memcpy(&m_PacketBuffer[0], &TempBuffer[0], m_nPacketBufferMark);
		}

		Receive_packet();
	}
}