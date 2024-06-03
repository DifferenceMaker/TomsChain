#pragma once
#define WIN32_LEAN_AND_MEAN // Prevent inclusion of winsock.h in Windows headers (FileDialog.h includes Windows.h)
#include "Renderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ImGui/imgui.h"

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <cassert>
#include <fstream>

#include "DBConn.h"
#include "Utils/FileDialog.h"
#include "Wallet.h"
#include "Transaction.h"
#include "Network/NetworkConn.h"
#include "Appsettings.hpp"


const int PK_LEN = 65; // 64 + 1

namespace client {
	static int selectedTab = 0;

	// Wallet Tab Variables
	static char inputPrivateKey[PK_LEN];
	static char inputPassword[64];
	static char walletName[24];
	static bool incorrectPasswordAttempt = false;
	static int selectedWallet = 0;

	// Transactions Tab
	static char receiversAddress[41];
	static char txAmount[16];
	static char gasPrice[16];
	static char gasLimit[16];
	/* selectedWallet is also used here */

	// Client Settings
	inline AppSettings clientSettings;

	class ClientModule {
	public:
		ClientModule();
		~ClientModule();

		void OnRender(); // Background
		void OnUpdate(double deltaTime);
		void OnImGuiRender(ImFont* _capitalFont);

	private:
		std::vector<Wallet> ClientWallets;

		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture;
		glm::mat4 m_Proj, m_View;
		double time = 0;

		/* Pictures / Icons */
		std::unique_ptr<Texture> wallet_texture;
		std::unique_ptr<Texture> blockchain_texture;
		std::unique_ptr<Texture> transactions_texture;
		std::unique_ptr<Texture> peers_texture;
		std::unique_ptr<Texture> client_texture;

		// File Dialog for Wallet Selection
		file::FileDialog fileDialog;
		std::string selectedFileStr;
		std::string filePathStr;

		// Connection Object
		std::unique_ptr<NetConn::Connection> m_Connection;	

		// DBConn Object
		std::shared_ptr<DBConn> db_;
		

	};
}