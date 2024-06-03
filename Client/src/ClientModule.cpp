#include "ClientModule.h"

namespace client {
	ClientModule::ClientModule() 
		:m_Proj(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)),
		m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))){

		float vertices[] = {
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // 0
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // 1
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // 2
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // 3
		};

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		m_VAO = std::make_unique<VertexArray>();
		m_VertexBuffer = std::make_unique<VertexBuffer>(vertices, 5 * 4 * sizeof(float));
		VertexBufferLayout layout;

		layout.Push<float>(3); // x, y and z
		layout.Push<float>(2); // texture coordinates

		m_VAO->AddBuffer(*m_VertexBuffer, layout);

		uint32_t indices[] = { // only used with Element draw; when cube drawn - glDrawArray with only vertices
			0, 1, 2,
			2, 3, 0
		};

		m_IndexBuffer = std::make_unique<IndexBuffer>(indices, 6);

		m_Shader = std::make_unique<Shader>("res/shaders/background.shader");
		m_Shader->Bind();

		m_Texture = std::make_unique<Texture>("res/textures/loginBackground.png");
		m_Shader->SetUniform1i("u_Texture", 0);

		wallet_texture = std::make_unique<Texture>("res/images/wallet_t.png");
		blockchain_texture = std::make_unique<Texture>("res/images/blockchain_t.png");
		transactions_texture = std::make_unique<Texture>("res/images/transactions_t.png");
		peers_texture = std::make_unique<Texture>("res/images/peers_t.png");
		client_texture = std::make_unique<Texture>("res/images/client_t.png");

		// Database connection setup
		db_ = std::make_shared<DBConn>();

		// Connection creation and setup
		m_Connection = std::make_unique<NetConn::Connection>(db_);
		m_Connection->startServer();
	}

	ClientModule::~ClientModule(){
	}

	void ClientModule::OnRender(){
		GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		Renderer renderer;
		m_Texture->Bind();

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
		glm::mat4 mvp = m_Proj * m_View * model;

		m_Shader->Bind();
		m_Shader->SetUniformMat4f("u_MVP", mvp);

		renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
	}

	void ClientModule::OnUpdate(double deltaTime){
		time = deltaTime;
	}

	void ClientModule::OnImGuiRender(ImFont* _capitalFont){
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImVec2 appWindowPos = mainViewport->Pos;
		ImVec2 appWindowSize = mainViewport->Size;

		// Calculate the window position relative to the application window
		ImVec2 windowPosition = ImVec2(
			appWindowPos.x + (appWindowSize.x - 400) / 2,
			appWindowPos.y + (appWindowSize.y - 400) / 2
		);


		// nav vajadziigs ne loginModule ne mainModule
		ImGui::SetNextWindowSize({ 1300, 750 });
		ImGui::SetNextWindowPos(ImVec2(0,0));
		if (ImGui::Begin("Client", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			
			// Navigation 
			if (ImGui::ImageButton((void*)(intptr_t)wallet_texture->getTextureID(), ImVec2(30, 30))) {
				selectedTab = 0;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Access Your Wallets");
			ImGui::SameLine();

			// Blockchain Tab
			if (ImGui::ImageButton((void*)(intptr_t)blockchain_texture->getTextureID(), ImVec2(30, 30))) {
				selectedTab = 1;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Browse The Blockchain");
			ImGui::SameLine();

			// Transactions Tab
			if (ImGui::ImageButton((void*)(intptr_t)transactions_texture->getTextureID(), ImVec2(30, 30))) {
				selectedTab = 2;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Send And Receive Transactions");
			ImGui::SameLine();

			// Peers Tab
			if (ImGui::ImageButton((void*)(intptr_t)peers_texture->getTextureID(), ImVec2(30, 30))) {
				selectedTab = 3;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("View And Manage Your Connected Peers");
			ImGui::SameLine();

			// Client Tab
			if (ImGui::ImageButton((void*)(intptr_t)client_texture->getTextureID(), ImVec2(30, 30))) {
				selectedTab = 4;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Manage Your Client And Settings");

			ImGui::Separator();

			// Tabs
			switch (selectedTab) {
			case 0:
				ImGui::PushFont(_capitalFont);
				ImGui::Text("Wallet");
				ImGui::PopFont();

				if(ImGui::Button("Create New Wallet")){
					ImGui::OpenPopup("Wallet Creation");
				};
				ImGui::SameLine();
				if(ImGui::Button("Add Existing Wallet")){
					bool result = fileDialog.openFile();
					switch (result) {
						case(TRUE): {
							selectedFileStr = fileDialog.getSelectedFile();
							filePathStr = fileDialog.getFilePath();		
							ImGui::OpenPopup("Wallet Password Confirmation");
							break;
						}
						case(FALSE): {
							printf("ENCOUNTERED AN ERROR: (%d)\n", GetLastError());
							break;
						}
					}
				};
				ImGui::SameLine();
				if (ImGui::Button("Recreate Existing Wallet")) {

				}if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Recreate an existing Wallet by providing it's private key and password");
				
				
				if (ClientWallets.size() != 0) {
					int totalFunds = 0;
					for (int i = 0; i < ClientWallets.size(); i++) {
						totalFunds += ClientWallets.at(i).getBalance();
					}
					std::stringstream totalFundsSTR;
					totalFundsSTR << "Total Funds : " << totalFunds << " UWC";
					ImGui::SameLine((appWindowPos.x + appWindowSize.x) - ImGui::CalcTextSize(totalFundsSTR.str().c_str()).x - 10.0f);
					ImGui::Text(totalFundsSTR.str().c_str());

					ImGui::Columns(2, nullptr, false);
					ImGui::SetColumnOffset(1, 340);

					if (ImGui::BeginChild("Wallet Tab", { 320, 600 }, false)) {

						for (int i = 0; i < ClientWallets.size(); i++) {
							Wallet currentWallet = ClientWallets.at(i);
							std::stringstream nonce; nonce << currentWallet.getNonce();
							std::stringstream balance; balance << currentWallet.getBalance();

							if(i == selectedWallet)
								ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 1));
							if(ImGui::BeginChild(i+1, { 300,100 }, true)){
								ImGui::Text(currentWallet.getShortenedAddress().c_str());
								ImGui::SameLine(300 - ImGui::CalcTextSize(nonce.str().c_str()).x - 10.0f);
								ImGui::Text(nonce.str().c_str());
								ImGui::Text(currentWallet.getWalletName().c_str());
								ImGui::Text("Wallet Funds: %s UWC", balance.str().c_str());

							}ImGui::EndChild();
							if (i == selectedWallet)
								ImGui::PopStyleColor();

							if (ImGui::IsItemHovered())
								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

							if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
							{
								selectedWallet = i;
							}
						}

					}ImGui::EndChild();

					ImGui::NextColumn();
					if (ImGui::BeginChild("Wallet Description", { 940, 600 }, false)) {
						ImGui::Text("Address: %s", ClientWallets.at(selectedWallet).getWalletAddressString().c_str());
					}ImGui::EndChild();



				} else {
					ImGui::SetCursorPos(ImVec2(520, 330));
					ImGui::Text("Your Wallets will be displayed here");
				}

				// Wallet Creation Modal

				ImGui::SetNextWindowSize({ 400, 400 });
				ImGui::SetNextWindowPos(windowPosition);
				if (ImGui::BeginPopupModal("Wallet Creation", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Name of the Wallet (max 24 characters)");
					ImGui::SetNextItemWidth(380);
					ImGui::InputText("##walletnameinput", walletName, sizeof(walletName));

					ImGui::Text("Password");
					ImGui::SetNextItemWidth(380);
					ImGui::InputText("##passwordinput", inputPassword, sizeof(inputPassword), ImGuiInputTextFlags_Password);

					if (ImGui::Button("Create an Account")) {
						Wallet newWallet(inputPassword, walletName);

						ClientWallets.emplace_back(newWallet);
						memset(inputPassword, 0, sizeof(inputPassword));
						memset(walletName, 0, sizeof(walletName));
						ImGui::CloseCurrentPopup();

					}
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, 360));

					
					if (ImGui::Button("Close")) {
						ImGui::CloseCurrentPopup();
						memset(inputPassword, 0, sizeof(inputPassword));
					}
					ImGui::EndPopup();
				}

				ImGui::SetNextWindowSize({ 460, 400 });
				ImGui::SetNextWindowPos({windowPosition.x - 30.0f, windowPosition.y});
				if (ImGui::BeginPopupModal("Wallet Password Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
					std::string userWalletAddress = selectedFileStr;
					
					[&userWalletAddress]() {
						for (int i = 0; i < 4; ++i) {
							if (!userWalletAddress.empty()) {
								userWalletAddress.pop_back();
							}
						}
					}();

					ImGui::Text("Wallet: %s", userWalletAddress.c_str());
					ImGui::Text("Enter the Wallet's Password");
					ImGui::SetNextItemWidth(440);
					if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
						ImGui::SetKeyboardFocusHere(0);
					ImGui::InputText("##passwordinput", inputPassword, sizeof(inputPassword), ImGuiInputTextFlags_Password);

					if (ImGui::Button("Sign in") || (ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused())) {
						Wallet newUserWallet(inputPassword, userWalletAddress, filePathStr);
						//std::cout << inputPassword << " " << userWalletAddress << " " << filePathStr << std::endl;;
						if (newUserWallet.accountLoggedIn) {
							ClientWallets.emplace_back(newUserWallet);
							incorrectPasswordAttempt = false;
							memset(inputPassword, 0, sizeof(inputPassword));
							ImGui::CloseCurrentPopup();
						}
						else {
							incorrectPasswordAttempt = true;
						}

					}
					if (incorrectPasswordAttempt)
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Incorrect Password - Try Again");					
					
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, 360));
					if (ImGui::Button("Close")) {
						ImGui::CloseCurrentPopup();
						incorrectPasswordAttempt = false;
						memset(inputPassword, 0, sizeof(inputPassword));
						selectedFileStr = "";
						filePathStr = "";
					}
					ImGui::EndPopup();
				}

				break;
			case 1:
				ImGui::PushFont(_capitalFont);
				ImGui::Text("Blockchain");
				ImGui::PopFont();

				break;
			case 2:
				ImGui::PushFont(_capitalFont);
				ImGui::Text("Transactions");
				ImGui::PopFont();

				if (ClientWallets.size() != 0) {
					ImGui::Columns(2, nullptr, false);

					ImGui::SetColumnOffset(1, 800);

					ImGui::Text("Receiver's address");
					ImGui::SetNextItemWidth(500.0f);
					ImGui::InputText("##ReceiversAddress", receiversAddress, sizeof(receiversAddress));

					ImGui::Text("Amount");
					ImGui::SameLine(190);
					ImGui::Text("Gas Price");
					ImGui::SameLine(370);
					ImGui::Text("Gas Limit");

					ImGui::SetNextItemWidth(140.0f);
					ImGui::InputText("##txAmount", txAmount, sizeof(txAmount));
					ImGui::SameLine(190);
					ImGui::SetNextItemWidth(140.0f);
					ImGui::InputText("##gasPrice", gasPrice, sizeof(gasPrice));
					ImGui::SameLine(370);
					ImGui::SetNextItemWidth(140.0f);
					ImGui::InputText("##gasLimit", gasLimit, sizeof(gasLimit));

					ImGui::Text("Choose Wallet");
					ImGui::SetNextItemWidth(500.0f);

					if (ImGui::BeginCombo("##walletSelection", ClientWallets.at(selectedWallet).getShortenedAddress().c_str())){

						for (int n = 0; n < ClientWallets.size(); n++) {
							bool is_selected = (selectedWallet == n);

							if (ImGui::Selectable(ClientWallets.at(n).getShortenedAddress().c_str(), is_selected)){
								selectedWallet = n;
								std::cout << "Selected wallet: " << n << std::endl;
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::Text("Available Funds:");
					ImGui::SameLine();
					ImGui::Text("%i UWC", ClientWallets.at(selectedWallet).getBalance());
					ImGui::SameLine();
					ImGui::Text("| Nonce:");
					ImGui::SameLine();
					ImGui::Text("%i", ClientWallets.at(selectedWallet).getNonce());

					
					if (ImGui::Button("Send Transaction")) {
						Wallet userWal = ClientWallets.at(selectedWallet);

						std::array<uint8_t, 20> receiversAddressArray;

						for (size_t i = 0, j = 0; i < 40; i += 2, ++j) {
							int hexValue = 0;
							sscanf_s(&receiversAddress[i], "%02x", &hexValue);

							receiversAddressArray[j] = static_cast<uint8_t>(hexValue);
						}

						// Create new Transaction object
						Transaction newTx((uint32_t)userWal.getNonce(), userWal.getWalletAddress(), receiversAddressArray, (uint64_t)std::atoi(gasPrice), (uint64_t)std::atoi(gasLimit), (uint64_t)std::atoi(txAmount));
						
						// Create v, r, s values for the Transaction
						std::pair<uint8_t, std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>>> vrs = userWal.signTransaction(newTx);
						
						// Assign created v, r, s values to the Transaction
						newTx.signTransaction(vrs.first, vrs.second.first, vrs.second.second);
						
						if (m_Connection->propogateTransaction(newTx)) {
							// Display good notifaction
							std::cout << "ClientModule: Transaction propogation returned true\n";
						}
						else {
							std::cout << "Bad\n";

						}

						/* TO DO 
						  - Show Receipt on the window.
						  - create P2P network so you can broadcast to the network.
						  - The Transaction gets broadcast to other people's pools 
						  - miners create blocks and then add to blockchain database and broadcast them.

						TO DO
						  P2P, Database, Consensus mechanism
						
						*/

					}if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Signs the transaction; Propogates to the network; Adds to a block");

					ImGui::NextColumn();
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.6));
					if (ImGui::BeginChild("Transaction Receipt", { 450, 600 }, true)) {
						ImGui::Text("Transaction Receipt");
						/*
						Display:
						  * Sender
						  * Recipient
						  * Amount
						  * Nonce (Current)
						  * Gas Fee (Limit: )
						  * Timestamp
						*/
					}ImGui::EndChild();
					ImGui::PopStyleColor();
				}
				else {
					ImGui::SetCursorPos(ImVec2(395, 330));
					ImGui::Text("You will be able to send transactions once you've added a Wallet.");

					ImGui::SetCursorPos(ImVec2(485, 360));
					ImGui::Text("(Go to Wallet tab to manage your Wallets)");
				}

				break;
			case 3:
				ImGui::PushFont(_capitalFont);
				ImGui::Text("Peers");
				ImGui::PopFont();
				/*
					- Display your currently connected peers 
					- Ability to see Certified Peer List

				*/

				break;
			case 4:
				ImGui::PushFont(_capitalFont);
				ImGui::Text("Client");
				ImGui::PopFont();
				ImGui::Text("Node Type: %s", clientSettings.node.type.c_str());
				/*
					- Ability to switch between node types (light / semi / full)
					- 
				*/

				break;
			}
		}ImGui::End();
	}
}