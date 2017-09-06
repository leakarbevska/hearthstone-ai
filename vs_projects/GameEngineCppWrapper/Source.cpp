#include "Header.h"

#include <chrono>
#include <sstream>

#include "UI/AIController.h"
#include "FlowControl/FlowController-impl.h"
#include "MCTS/TestStateBuilder.h"
#include "Cards/PreIndexedCards.h"

namespace GameEngineCppWrapper
{
	class GameEngineImpl {
	public:
		GameEngineImpl() : controller_(), output_message_callback_(nullptr) {}

		int Initialize() {
			try {
				if (!Cards::Database::GetInstance().Initialize("cards.json")) {
					Log("Failed to load cards.json.");
					return -1;
				}
				Cards::PreIndexedCards::GetInstance().Initialize();
			}
			catch (...) {
				Log("Failed to load cards.json.");
				return -1;
			}
			
			Log("Successfully load cards.json.");
			return 0;
		}
		
		int Run(int seconds, int threads) {
			auto run_until = std::chrono::steady_clock::now() +
				std::chrono::seconds(seconds);

			long long last_report_rest_sec = -1;
			auto continue_checker = [&]() {
				auto now = std::chrono::steady_clock::now();
				if (now > run_until) return false;

				auto rest_sec = std::chrono::duration_cast<std::chrono::seconds>(run_until - now).count();
				if (rest_sec != last_report_rest_sec) {
					{
						std::stringstream ss;
						ss << "Rest seconds: " << rest_sec;
						Log(ss.str());
					}

					{
						std::stringstream ss;
						auto iterations = controller_.GetStatistic().GetSuccededIterates();
						ss << "Iterations: " << iterations;
						Log(ss.str());
					}

					last_report_rest_sec = rest_sec;
				}
				return true;
			};

			auto start_board_getter = [](int seed) -> state::State {
				return TestStateBuilder().GetState(seed);
			};

			try {
				controller_.Run(continue_checker, threads, start_board_getter);
			}
			catch (...) {
				return -1;
			}

			return 0;
		}

		void SetOutputMessageCallback(OutputMessageCallback cb)
		{
			output_message_callback_ = cb;
		}

	private:
		void Log(std::string const& msg)
		{
			if (output_message_callback_) {
				output_message_callback_(msg);
			}
		}

	private:
		ui::AIController controller_;
		OutputMessageCallback output_message_callback_;
	};

	GameEngine::GameEngine() : impl_(nullptr) {
		impl_ = new GameEngineImpl();
	}

	int GameEngine::Initialize() const
	{
		return impl_->Initialize();
	}

	void GameEngine::SetOutputMessageCallback(OutputMessageCallback cb)
	{
		return impl_->SetOutputMessageCallback(cb);
	}

	int GameEngine::Run(int seconds, int threads) { return impl_->Run(seconds, threads); }
}