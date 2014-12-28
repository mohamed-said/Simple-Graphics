// Graphics_SDDA.cpp : Defines the entry point for the application.
//
// apply simple DDA algorithm

#include "stdafx.h"
#include "Graphics.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <fstream>
#include <time.h>
#include <conio.h>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
bool b_SDDA_line = false;
bool b_midPoint_Line = false;
bool b_midPointCircle = false;
bool b_diamond = false;

struct special {
	int x_coordinate;
	int y_coordinate;
};
struct MY_POINT {
	string algorithm;
	int fx, fy, sx, sy;
};

vector<special> draw_special;
vector<MY_POINT> my_undo;
vector<MY_POINT> my_redo;
//vector<MY_POINT> data_to_be_saved;

HDC global_hdc;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

double first_x, first_y, second_x, second_y; // coordinates variables -> 2 points
int ctr = 0; // counter to points

ofstream logFile;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
int					round(double);
void				my_swap(double&, double&);
bool				isDiagonal();
void				initialize_redo_and_undo();
void				simple_DDA(HDC);
void				mid_point_algorithm(HDC);
void				mid_point_circle(HDC);
void				load_project();
void				save_project();
void				draw_diamond(HDC);
// -------------------------------------------------------------

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GRAPHICS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHICS));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//

ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHICS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GRAPHICS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   HWND hWnd;
   hInst = hInstance; // Store instance handle in our global variable
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

 // we work here 

int round(double n) {
	return int(n + 0.5);
}

void my_swap(double& x, double& y) {
	double temp = x;
	x = y;
	y = temp;
}

bool isDiagonal() {
	return (second_y - first_y) == (second_x - first_x);
}

void initialize_redo_and_undo() {
	MY_POINT MP;
	MP.fx = first_x;
	MP.fy = first_y;
	MP.sx = second_x;
	MP.sy = second_y;
	my_undo.push_back(MP);
	my_redo.push_back(MP);
}

void simple_DDA(HDC hdc) {
	COLORREF RGB(0, 0, 0);
	double slope = (second_y - first_y) / (second_x - first_x);
	double y_intercept = first_y - (slope * first_x);
	int x_coordinate;
	int y_coordinate;
	// special cases
	if (first_x == second_x) { // vertical line
		if (first_y > second_y) my_swap(first_y, second_y);
		for (x_coordinate = first_x, y_coordinate = first_y;
			y_coordinate <= second_y;
			y_coordinate++) {
				SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	} else if (first_y == second_y) { // horizontal lines
		for (x_coordinate = first_x, y_coordinate = first_y;
			x_coordinate <= second_x; x_coordinate++) {
				SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	} else if (isDiagonal()) { // diagonal line
		if (first_x > second_x) {
			my_swap(first_x, second_x);
			my_swap(first_y, second_y);
			for (x_coordinate = first_x, y_coordinate = first_y;
				x_coordinate <= second_x;
				x_coordinate++, y_coordinate++) {
					SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
			}
		}
	}
	if (abs(slope) <= 1) {
		if (first_x > second_x) my_swap(first_x, second_x);	
		for (x_coordinate = (int)first_x; x_coordinate <= (int)second_x; x_coordinate++) {
			y_coordinate = (slope * x_coordinate) + y_intercept;
			SetPixel(hdc, x_coordinate, round(y_coordinate), COLORREF RGB(0, 0, 0));
		}
	} else { // absolute value of slope is more than '1'
		if (first_y > second_y) my_swap(first_y, second_y);
		for (y_coordinate = (int)first_y; y_coordinate <= (int)second_y; y_coordinate++) {
			x_coordinate = (y_coordinate - y_intercept) / slope;
			SetPixel(hdc, round(x_coordinate), y_coordinate, COLORREF RGB(0, 0, 0));
		}
	}
	initialize_redo_and_undo();
}

void mid_point_algorithm(HDC hdc) {
	logFile.open("log.txt", ios::app);
	double d_old, d_NE, d_E;
	double slope = (second_y - first_y) * 1.0 / (second_x - first_x);
	int x_coordinate;
	int y_coordinate;
	
	if (first_x == second_x) { // vertocal line
		if (first_y > second_y) my_swap(first_y, second_y);
		for (x_coordinate = first_x, y_coordinate = first_y;
			y_coordinate <= second_y;
			y_coordinate++) {
				SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
		return;
	}

	if (first_y == second_y) { // horizontal line
		if (first_x > second_x) my_swap(first_x, second_x);
		for (x_coordinate = first_x, y_coordinate = first_y;
			x_coordinate <= second_x;
			x_coordinate++) {
				SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
		return;
	}
	
	//logFile << slope << " " << d_old << " " << d_NE << " " << d_E << endl;
	if (slope >= 0 && slope <= 1) {
		d_old = (2 * abs(second_y - first_y)) - abs(second_x - first_x);
		d_NE = (2 * abs(second_y - first_y) - 2 * abs(second_x - first_x));
		d_E = (2 * abs(second_y - first_y));
		if (first_x > second_x) {
			my_swap(first_x, second_x);
			my_swap(first_y, second_y);
		}
		x_coordinate = first_x;
		y_coordinate = first_y;
		SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		while (x_coordinate < second_x) {
			if (d_old <= 0) {
				d_old += d_E;
				x_coordinate++;
			} else {
				d_old += d_NE;
				x_coordinate++;
				y_coordinate++;
			}
			SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	} else if (slope > 1) {
		d_old = (2 * abs(second_x - first_x)) - abs(second_y - first_y);
		d_NE = (2 * abs(second_x - first_x) - 2 * abs(second_y - first_y));
		d_E = (2 * abs(second_x - first_x));
		if (first_y > second_y) {
			my_swap(first_x, second_x);
			my_swap(first_y, second_y);
		}
		x_coordinate = first_x;
		y_coordinate = first_y;
		while (y_coordinate < second_y) {
			if (d_old <= 0) {
				d_old += d_E;
				y_coordinate++;
			} else {
				d_old += d_NE;
				y_coordinate++;
				x_coordinate++;
			}
			SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	} else if (slope <= 0 && slope >= -1) {
		d_old = (2 * abs(second_y - first_y)) - abs(second_x - first_x);
		d_NE = (2 * abs(second_y - first_y) - 2 * abs(second_x - first_x));
		d_E = (2 * abs(second_x - first_x));
		if (first_x > second_x) {
			my_swap(first_x, second_x);
			my_swap(first_y, second_y);
		}
		x_coordinate = first_x;
		y_coordinate = first_y;
		SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		while (x_coordinate < second_x) {
			if (d_old <= 0) {
				d_old += d_E;
				x_coordinate++;
			} else {
				d_old += d_NE;
				x_coordinate++;
				y_coordinate--;
			}
			SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	} else if (slope < -1 ) {
		d_old = (2 * abs(second_x - first_x)) - abs(second_y - first_y);
		d_NE = (2 * abs(second_x - first_x) - 2 * abs(second_y - first_y));
		d_E = (2 * abs(second_x - first_x));
		if (first_y > second_y) {
			my_swap(first_x, second_x);
			my_swap(first_y, second_y);
		}
		x_coordinate = first_x;
		y_coordinate = first_y;
		while (y_coordinate < second_y) {
			if (d_old <= 0) {
				d_old += d_E;
				y_coordinate++;
			} else {
				d_old += d_NE;
				y_coordinate++;
				x_coordinate--;
			}
			SetPixel(hdc, x_coordinate, y_coordinate, COLORREF RGB(0, 0, 0));
		}
	}
	initialize_redo_and_undo();
}

void draw_8_points(HDC hdc, int x, int y, COLORREF RGB) {

	int xx = first_x;
	int yy = first_y;
	SetPixel(hdc, xx + x, yy + y, RGB(0, 0, 0));
	SetPixel(hdc, xx + y, yy + x, RGB(0, 0, 0));
	SetPixel(hdc, xx - y, yy + x, RGB(0, 0, 0));
	SetPixel(hdc, xx - x, yy + y, RGB(0, 0, 0));
	SetPixel(hdc, xx - x, yy - y, RGB(0, 0, 0));
	SetPixel(hdc, xx - y, yy - x, RGB(0, 0, 0));
	SetPixel(hdc, xx + y, yy - x, RGB(0, 0, 0));
	SetPixel(hdc, xx + x, yy - y, RGB(0, 0, 0));

}

void mid_point_circle(HDC hdc) {
	logFile.open("log.txt", ios::app);
	logFile << "method called successfully\n";
	COLORREF RGB(0, 0, 0);
	logFile << "RGB defined successfully\n";
	int R = hypot(abs(second_x - first_x), abs(second_y - first_y));
	int x_coordinate = 0;
	int y_coordinate = R;
	double h = 1 - R;
	logFile << "variables set successfully\n";
	logFile << R << " " << x_coordinate << " " << y_coordinate << " " << h << endl;
	draw_8_points(hdc, x_coordinate, y_coordinate, RGB(0, 0, 0)); 
	while (y_coordinate > x_coordinate) {
		if (h < 0) {
			h += (x_coordinate << 1) + 3;
		} else {
			h += ((x_coordinate - y_coordinate) << 1) + 5;
			--y_coordinate;
		}
		++x_coordinate;
		draw_8_points(hdc, x_coordinate, y_coordinate, RGB(0, 0, 0));
	}
}


void optimized_mid_point_circle(HDC hdc) {
	COLORREF RGB(0, 0, 0);
	int R = hypot(abs(second_x - first_x), abs(second_y - first_y));
	int x_coordinate = 0;
	int y_coordinate = R;
	double h = 1 - R;
	int delta_E = 3;
	int delta_SE = -(R << 1) + 5;
	draw_8_points(hdc, x_coordinate, y_coordinate, RGB(0, 0, 0));
	while (y_coordinate > x_coordinate) {
		if (h < 0) {
			h += delta_E;
			delta_E += 2;
			delta_SE += 2;
		} else {
			h += delta_SE;
			delta_E += 2;
			delta_SE += 4;
			--y_coordinate;
		}
		++x_coordinate;
		draw_8_points(hdc, x_coordinate, y_coordinate, RGB(0, 0, 0));
	}
}

void draw_diamond(HDC hdc) {
	vector<MY_POINT> pnts;
	int x_coordinate;
	int y_coordinate;
	int horizontal_diameter = abs(first_x - second_x) * 2;
	int vertical_diameter = abs(first_y - second_y) * 2;
	if (first_x > second_x) {
		my_swap(first_x, second_x);
		my_swap(first_y, second_y);
	}

	special p1, p2, p3, p4;
	p1.x_coordinate = first_x;
	p1.y_coordinate = first_y;
	draw_special.push_back(p1);

	p2.x_coordinate = second_x;
	p2.y_coordinate = second_y;
	draw_special.push_back(p2);

	p3.x_coordinate = first_x + horizontal_diameter;
	p3.y_coordinate = first_y;
	draw_special.push_back(p3);

	p4.x_coordinate = second_x;
	p4.y_coordinate = second_y - vertical_diameter;
	draw_special.push_back(p4);

	for (int i = 0; i < 4; i++) {
		if (i == 3) {
			first_x = draw_special[i].x_coordinate;
			first_y = draw_special[i].y_coordinate;
			second_x = draw_special[0].x_coordinate;
			second_y = draw_special[0].y_coordinate;
		} else {
			first_x = draw_special[i].x_coordinate;
			first_y = draw_special[i].y_coordinate;
			second_x = draw_special[i + 1].x_coordinate;
			second_y = draw_special[i + 1].y_coordinate;	
		}
		simple_DDA(hdc);
	}
	draw_special.clear();
}

void load_project() {
	// 1 - a windows pops up to choos the file
	// 2 - load the chosen file and start reading
	// ifstream infile;
	// infile.open();
	//string algorithm;
/*	while (!infile.eof()) {
		infile >> algorithm;
		infile >> first_x;
		infile >> first_y;
		infile >> second_x;
		infile >> second_y;
		if (algorithm == "SDDA")
			simple_DDA(global_hdc);
		else if (algorithm == "MPL")
			mid_point_algorithm(global_hdc);
		else if (algorithm == "OMPC")
			optimized_mid_point_circle(global_hdc);
	}
*/
}

void save_project() {
	// MessageBox(); // choose the file name
	// ofstream onfile;
	// onfile.open("filename".c_str(), ios::app);
	// get file name from the message box and convert it to c_str()
	// to create a file with the specified name from the user
/*	for (int i = 0; i < (int)data_to_be_saved.size(); ++i) {
		onfile << data_to_be_saved[i].algorithm << " ";
		onfile << data_to_be_saved[i].fx << " ";
		onfile << data_to_be_saved[i].fy << " ";
		onfile << data_to_be_saved[i].sx << " ";
		onfile << data_to_be_saved[i].sy << " ";
		onfile << "\n";
	}
*/
}


void save_data(string algo) {
/*	
	MY_POINT temp;
	temp.algorithm = algo;
	temp.fx = first_x;
	temp.fy = first_y;
	temp.sx = second_x;
	temp.sy = second_y;
	data_to_be_saved.push_back(temp);
*/
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	// global_hdc = ps.hdc; // run time error O_o
	HDC hdc;
	switch (message) {
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			//////////////
		case IDM_SDDA:
			b_SDDA_line = true;
			b_midPoint_Line = false;
			b_midPointCircle = false;
			InvalidateRect(hWnd, NULL, false);
			break;
		case IDM_MidPoint:
			b_midPoint_Line = true;
			b_SDDA_line = false;
			b_midPointCircle = false;
			InvalidateRect(hWnd, NULL, false);
			break;
		case IDM_MidPointCircle:
			b_midPointCircle = true;
			b_SDDA_line = false;
			b_midPoint_Line = false;
			InvalidateRect(hWnd, NULL, false);
			break;
		case IDM_DIAMOND:
			b_midPointCircle = false;
			b_SDDA_line = false;
			b_midPoint_Line = false;
			b_diamond = true;
			InvalidateRect(hWnd, NULL, false);		
			break;
		case IDM_UNDO:
			my_undo.pop_back();
			break;
		case IDM_REDO:
			if (b_SDDA_line && (int)my_redo.size()) {
				for (int i = 0; i < (int)my_redo.size(); i++) {
					first_x = my_redo[i].fx;
					first_y = my_redo[i].fy;
					second_x = my_redo[i].sx;
					second_y = my_redo[i].sy;
					InvalidateRect(hWnd, NULL, false);
					simple_DDA(ps.hdc);
				}
			} else if (b_midPoint_Line && (int)my_redo.size()) {
				for (int i = 0; i < (int)my_redo.size(); i++) {
					first_x = my_redo[i].fx;
					first_y = my_redo[i].fy;
					second_x = my_redo[i].sx;
					second_y = my_redo[i].sy;
					InvalidateRect(hWnd, NULL, false);
					mid_point_algorithm(ps.hdc);
				}
			}
			break;
		case IDM_LOAD:
			//load_project();
			break;
		case IDM_SAVE:
			// save_project();
			break;
		case IDM_CHOOSE_COLOR:
			// to be implemented
			break;
			///////////// 
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	
		
	/*added code start*/
	case WM_LBUTTONDOWN:
		if (ctr == 0) {
			first_x = LOWORD(lParam);
			first_y = HIWORD(lParam);
			ctr++;
		} else if (ctr == 1) {
			second_x = LOWORD(lParam);
			second_y = HIWORD(lParam);
			ctr++;
			InvalidateRect(hWnd, NULL, false);
		} break;

	/*added code end*/

	case WM_PAINT:
		if (ctr == 2) {
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			//MoveToEx(ps.hdc, first_x, first_y, NULL); // added
			if (b_SDDA_line) {
				simple_DDA(ps.hdc); // added /*simple digital diffrential algorithm*/
				//save_data("SDDA");
			} else if (b_midPoint_Line) {
				mid_point_algorithm(ps.hdc);
				//save_data("MPL");
			} else if (b_midPointCircle) {
				optimized_mid_point_circle(ps.hdc);
				//save_data("OMPC");
			} else if (b_diamond) {
				draw_diamond(ps.hdc);
			}
			//LineTo(ps.hdc, second_x, second_y); // added
			EndPaint(hWnd, &ps);
			ctr = 0;
		} break;
	
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
