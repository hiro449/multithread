

#include<windows.h>

#include<fstream>
#include<stdio.h>
#include"resource.h"
#include<process.h>
#include<memory>
#include<iostream>



static int number = 0;
//クリティカルセクション



class thread {


	HANDLE hNamedPipe;
	HWND         hDlg;
	HANDLE hStopEvent;//キャンセルのイベントx

	unsigned ThreadProc() {

		CRITICAL_SECTION critical;

		InitializeCriticalSection(&critical);

		EnterCriticalSection(&critical);

		number++;
		//クリティカルセクション
		LeaveCriticalSection(&critical);



		if (std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> hEvent
			{
			  CreateEventW(nullptr,true,false, nullptr), CloseHandle
			})
		{

			//イベントを作る

			OVERLAPPED ov
			{
				/*ULONG_PTR Internal;    */0
			   ,/*ULONG_PTR InternalHigh;*/0
			   ,/*       union           */{}
			   ,/*HANDLE  hEvent;        */hEvent.get()
			};

			if (ConnectNamedPipe(hNamedPipe, &ov)) {//ここでthisがエラー

				PostMessageW(hDlg, WM_COMMAND, IDOK, 0);
				std::cout << "パイプは直ぐに成功しました\n";
				return EXIT_SUCCESS;

				//いきなり成功した場合　何を返すのか関数の戻り値を見る
			}//イベント持ち
			else {

				const auto e = GetLastError();

				if (e == ERROR_IO_PENDING) {

					HANDLE handles[] =
					{
						hEvent.get(), hStopEvent
					};


					switch (WaitForMultipleObjects(_countof(handles), handles, false, INFINITE))
					{

					case WAIT_OBJECT_0 + 0: PostMessageW(hDlg, WM_COMMAND, IDOK, 0); EXIT_SUCCESS;//接続イベント
					case WAIT_OBJECT_0 + 1: EXIT_SUCCESS;//中止イベント



					case WAIT_FAILED:
						return EXIT_FAILURE;

					default:
						PostMessageW(hDlg, WM_COMMAND, IDABORT, 0);
						return EXIT_FAILURE;
					}
					//非同期で直ぐに成功した場合　待ち状態を続ける


				}
				else
					return EXIT_FAILURE;
				//関数が失敗した場合


			}


		}
		return EXIT_FAILURE;
	}

	//裏スレッド処理　待ち状態　表で待ち状態解除 
	//threadのタイミングでコネクトのハンドル終了

	//イベントは停止、接続の二つを作る

public:

	thread
	(
		HANDLE   hNamedPipe,
		HWND           hDlg,
		HANDLE    hStopEvent
	)
		:hNamedPipe{ hNamedPipe }, hDlg{ hDlg }, hStopEvent{ hStopEvent }
	{

	}

	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> CreatThread() {

		return std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)>
		{ HANDLE(_beginthreadex
		(
			/* _In_opt_  void* _Security,                       */nullptr
			,/* _In_      unsigned                 _StackSize,   */0
			,/* _In_      _beginthreadex_proc_type _StartAddress,*/[]
			(void* pvThis)->unsigned
			{

				return std::unique_ptr<thread>(static_cast<thread*>(pvThis))->ThreadProc();
		//キャンセルが推されたらセットイベント
		//ボタン実装　セットイベント 待ち解除　非同期でキャンセル
		//イベントオブジェクトを作る
		//GUIに処理を戻す？
		//おもてスレッド停止

			}//スレッド命令が流れる
			,/* _In_opt_  void* _ArgList,                        */this
				,/* _In_      unsigned                 _InitFlag,    */0
				,/* _Out_opt_ unsigned* _ThrdAddr                    */nullptr
				)), CloseHandle };//スレッドのハンドル




	}

};

class Dlg {
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> hStopEvent;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)>    hThread;
	HANDLE hNamedPipe;

	static Dlg* StopHandle(HWND hWnd) {
		return reinterpret_cast<Dlg*>(GetWindowLongPtrW(hWnd, DWLP_USER));
	}


	INT_PTR Init(HWND hWnd) {

		auto backTread = std::make_unique<thread>
			(
				/*HANDLE hNamedPipe,  */hNamedPipe
				,/*HWND hDlg,          */hWnd
				,/*HANDLE && hStopEvent*/hStopEvent.get()
				);//これもユニークで ストップイベントを入れる

		if (auto h = backTread->CreatThread()) {
			this->hThread = std::move(h);
			backTread.release();
			//スレッドハンドルをどうするのか
		}
		return true;

	}

public:
	Dlg(HANDLE                                                                     hNamedPipe
		, std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)>&& hStopEvent
	) noexcept
		:hNamedPipe{ hNamedPipe }
		, hStopEvent{ std::move(hStopEvent) }
		, hThread{ nullptr , CloseHandle }
	{
	}


	INT_PTR ConnectNamedPipe() {



		return DialogBoxParamW
		(
			/* _In_opt_ HINSTANCE hInstance, */nullptr
			,/* _In_ LPCWSTR lpTemplateName,  */MAKEINTRESOURCE(IDD_DIALOG1)
			,/* _In_opt_ HWND hWndParent,     */nullptr
			,/* _In_opt_ DLGPROC lpDialogFunc,*/[]
			(HWND     hWnd,
				UINT  message,
				WPARAM wParam,
				LPARAM lParam
				)->INT_PTR
			{
				switch (message)
				{
				case WM_INITDIALOG:
					SetWindowLongPtrW(hWnd, DWLP_USER, lParam);
					return reinterpret_cast<Dlg*>(lParam)->Init(hWnd);//ここっぽい

				case WM_COMMAND:
					switch (LOWORD(wParam))
					{

					case IDOK:
						return true;

					case IDCANCEL:

						SetEvent(StopHandle(hWnd)->hStopEvent.get());//ここでデストラクトを入れる ストップ
						EndDialog(hWnd, IDCANCEL);
						return true;

					case IDABORT:
						EndDialog(hWnd, IDABORT);
						return true;

					default:
						return false;
					}

					return true;

				case WM_DESTROY:
					return true;

				default:
					return false;
				}
			}
			,/* _In_ LPARAM dwInitParam       */LPARAM(this)
				);




	}

};



int main() {


	auto hNamedPipe = CreateNamedPipeW
	(
		/* _In_ LPCWSTR lpName,                               */LR"(\\.\pipe\test)"
		,/* _In_ DWORD dwOpenMode,                             */PIPE_ACCESS_INBOUND
		,/* _In_ DWORD dwPipeMode,                             */0
		,/* _In_ DWORD nMaxInstances,                          */1
		,/* _In_ DWORD nOutBufferSize,                         */0
		,/* _In_ DWORD nInBufferSize,                          */0
		,/* _In_ DWORD nDefaultTimeOut,                        */NMPWAIT_USE_DEFAULT_WAIT
		,/* _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes*/nullptr
	);


	if (hNamedPipe != INVALID_HANDLE_VALUE) {

		if (std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> hStopEvent
			{ CreateEventW(nullptr,true,false, nullptr), CloseHandle })
		{

			const auto result = Dlg(hNamedPipe, std::move(hStopEvent)).ConnectNamedPipe();

			if (result == IDOK) {
				for (;;) {

					char ch[256];
					DWORD cbread;
					//待機の停め方
					if (ReadFile(hNamedPipe, ch, sizeof ch, &cbread, nullptr)) {
						fwrite(ch, cbread, 1, stdout);
					}
					else
						break;

				}

			}
		}
		CloseHandle(hNamedPipe);
	}

}