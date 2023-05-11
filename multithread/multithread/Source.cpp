

#include<windows.h>

#include<fstream>
#include<stdio.h>
#include"resource.h"
#include<process.h>
#include<memory>
#include<iostream>



static int number = 0;
//�N���e�B�J���Z�N�V����



class thread {


	HANDLE hNamedPipe;
	HWND         hDlg;
	HANDLE hStopEvent;//�L�����Z���̃C�x���gx

	unsigned ThreadProc() {

		CRITICAL_SECTION critical;

		InitializeCriticalSection(&critical);

		EnterCriticalSection(&critical);

		number++;
		//�N���e�B�J���Z�N�V����
		LeaveCriticalSection(&critical);



		if (std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> hEvent
			{
			  CreateEventW(nullptr,true,false, nullptr), CloseHandle
			})
		{

			//�C�x���g�����

			OVERLAPPED ov
			{
				/*ULONG_PTR Internal;    */0
			   ,/*ULONG_PTR InternalHigh;*/0
			   ,/*       union           */{}
			   ,/*HANDLE  hEvent;        */hEvent.get()
			};

			if (ConnectNamedPipe(hNamedPipe, &ov)) {//������this���G���[

				PostMessageW(hDlg, WM_COMMAND, IDOK, 0);
				std::cout << "�p�C�v�͒����ɐ������܂���\n";
				return EXIT_SUCCESS;

				//�����Ȃ萬�������ꍇ�@����Ԃ��̂��֐��̖߂�l������
			}//�C�x���g����
			else {

				const auto e = GetLastError();

				if (e == ERROR_IO_PENDING) {

					HANDLE handles[] =
					{
						hEvent.get(), hStopEvent
					};


					switch (WaitForMultipleObjects(_countof(handles), handles, false, INFINITE))
					{

					case WAIT_OBJECT_0 + 0: PostMessageW(hDlg, WM_COMMAND, IDOK, 0); EXIT_SUCCESS;//�ڑ��C�x���g
					case WAIT_OBJECT_0 + 1: EXIT_SUCCESS;//���~�C�x���g



					case WAIT_FAILED:
						return EXIT_FAILURE;

					default:
						PostMessageW(hDlg, WM_COMMAND, IDABORT, 0);
						return EXIT_FAILURE;
					}
					//�񓯊��Œ����ɐ��������ꍇ�@�҂���Ԃ𑱂���


				}
				else
					return EXIT_FAILURE;
				//�֐������s�����ꍇ


			}


		}
		return EXIT_FAILURE;
	}

	//���X���b�h�����@�҂���ԁ@�\�ő҂���ԉ��� 
	//thread�̃^�C�~���O�ŃR�l�N�g�̃n���h���I��

	//�C�x���g�͒�~�A�ڑ��̓�����

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
		//�L�����Z���������ꂽ��Z�b�g�C�x���g
		//�{�^�������@�Z�b�g�C�x���g �҂������@�񓯊��ŃL�����Z��
		//�C�x���g�I�u�W�F�N�g�����
		//GUI�ɏ�����߂��H
		//�����ăX���b�h��~

			}//�X���b�h���߂������
			,/* _In_opt_  void* _ArgList,                        */this
				,/* _In_      unsigned                 _InitFlag,    */0
				,/* _Out_opt_ unsigned* _ThrdAddr                    */nullptr
				)), CloseHandle };//�X���b�h�̃n���h��




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
				);//��������j�[�N�� �X�g�b�v�C�x���g������

		if (auto h = backTread->CreatThread()) {
			this->hThread = std::move(h);
			backTread.release();
			//�X���b�h�n���h�����ǂ�����̂�
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
					return reinterpret_cast<Dlg*>(lParam)->Init(hWnd);//�������ۂ�

				case WM_COMMAND:
					switch (LOWORD(wParam))
					{

					case IDOK:
						return true;

					case IDCANCEL:

						SetEvent(StopHandle(hWnd)->hStopEvent.get());//�����Ńf�X�g���N�g������ �X�g�b�v
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
					//�ҋ@�̒�ߕ�
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