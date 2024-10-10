#include "MainForm.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "BitmapConverter.h"
#define SAVE_ALL

struct ImageInfo
{
	std::vector<cv::Rect> lstAllRectangles;
	int noAllPoles;
	std::wstring fileName = L"";
	cv::Rect subROI = cv::Rect(0,0,0,0);
	std::vector<std::tuple<int, cv::Point, cv::Point>> lstAllPolesPosition;
};


std::vector<ImageInfo> lstImageInfo;
System::Drawing::Point cellEdit(0,0);
float zoomScale = 1;
float zoomStep = 0.1;

XavisTech::MainForm::MainForm(void)
	: m_isEdiable(true)
	, m_isMouseDown(false)
	, m_totalImage(0)
	, m_imageIdx(0)
	, m_scale(0)
	, m_alternateColor(AlternateColor::UP_DOWN)
	, m_poleShape(Shape::POINT)
	, m_picBoxOrgSize (System::Drawing::Rectangle(0, 0, 0, 0))
	, m_centerPanel(System::Drawing::Point(0, 0))
	, m_offsetPoint(System::Drawing::Point(0, 0))
{
	InitializeComponent();
	
	//
	//TODO: Add the constructor code here
	//
	m_centerPanel = System::Drawing::Point(panel3->Location.X + panel3->Width/2, panel3->Location.Y + panel3->Height / 2);
	
	this->panel3->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox_MouseWheel);
	//Load Typical Value
	this->dataGridView1->Columns->Add("Column0", "Index");
	this->dataGridView1->Columns->Add("Column1", "X,Y,Width,Height");
	this->dataGridView1->Columns->Add("Column2", "GrayAvg");
	this->dataGridView1->Columns[0]->Width = this->dataGridView1->Width * 0.2;
	this->dataGridView1->Columns[1]->Width = this->dataGridView1->Width * 0.5;
	this->dataGridView1->Columns[2]->Width = this->dataGridView1->Width * 0.3;
	this->dataGridView1->Columns[0]->ReadOnly = true;
	this->dataGridView1->Columns[1]->ReadOnly = true;
	this->dataGridView1->Columns[2]->ReadOnly = true;
	this->dataGridView1->Columns[0]->SortMode = DataGridViewColumnSortMode::NotSortable;
	this->dataGridView1->Columns[1]->SortMode = DataGridViewColumnSortMode::NotSortable;
	this->dataGridView1->Columns[2]->SortMode = DataGridViewColumnSortMode::NotSortable;
}

XavisTech::MainForm::~MainForm()
{
	if (components)
	{
		delete components;
	}
}


void XavisTech::MainForm::caculateScaleOffset()
{
	float diffRatio = (float)m_image->Width / panel3->Width - (float)m_image->Height / panel3->Height;
	if (diffRatio > 0)
	{
		m_scale = (float)panel3->Width / m_image->Width;
		m_offsetPoint.X = 0;
		m_offsetPoint.Y = (int)(panel3->Height / 2 - m_image->Height * m_scale / 2);
	}
	else
	{
		m_scale = (float)panel3->Height / m_image->Height;
		m_offsetPoint.X = (int)(panel3->Width / 2 - m_image->Width * m_scale / 2);
		m_offsetPoint.Y = 0;
	}

	pictureBox->Location = m_offsetPoint;
	pictureBox->Width = m_image->Width * m_scale;
	pictureBox->Height = m_image->Height * m_scale;
	m_picBoxOrgSize = System::Drawing::Rectangle(pictureBox->Location.X, pictureBox->Location.Y, pictureBox->Width, pictureBox->Height);
}

System::Void XavisTech::MainForm::pictureBox_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	m_isMouseDown = true;
	if (m_totalImage <= 0) return;
	
	m_isMouseDown = true;  // Đánh dấu chuột đang được giữ

	if (e->Button == System::Windows::Forms::MouseButtons::Left)  // Kiểm tra nút chuột trái
	{
		// Lưu tọa độ bắt đầu khi nhấn chuột
		m_rectStartPoint = e->Location;
	}
}

void XavisTech::MainForm::Update()
{
	if (m_totalImage <= 0)
	{
		Reset();
		return;
	}

	ImageInfo* currentImageInfor = &lstImageInfo[m_imageIdx];

	// Quản lý danh sách các hình chữ nhật
	int numberOfRectangles = currentImageInfor->lstAllRectangles.size();

	// Kiểm tra nếu danh sách hình chữ nhật rỗng, khởi tạo dữ liệu cho dataGridView và lstAllRectangles
	if (currentImageInfor->lstAllRectangles.empty())
	{
		for (int i = 0; i < numberOfRectangles; i++)
		{
			this->dataGridView1->Rows->Add(i, "(-1,-1)", "(0,0)");
			currentImageInfor->lstAllRectangles.push_back(cv::Rect(-1, -1, 0, 0));
		}
	}
	else
	{
		// Nếu danh sách các hình chữ nhật thay đổi, cập nhật dataGridView và lstAllRectangles
		if (currentImageInfor->lstAllRectangles.size() < numberOfRectangles)
		{
			int numAdd = numberOfRectangles - currentImageInfor->lstAllRectangles.size();
			for (int i = 0; i < numAdd; i++)
			{
				this->dataGridView1->Rows->Add(currentImageInfor->lstAllRectangles.size(), "(-1,-1)", "(0,0)");
				currentImageInfor->lstAllRectangles.push_back(cv::Rect(-1, -1, 0, 0));
			}
		}
		else if (currentImageInfor->lstAllRectangles.size() > numberOfRectangles)
		{
			int numErase = currentImageInfor->lstAllRectangles.size() - numberOfRectangles;
			for (int i = 0; i < numErase; i++)
			{
				this->dataGridView1->Rows->RemoveAt(this->dataGridView1->Rows->Count - 2);
			}
			currentImageInfor->lstAllRectangles.erase(currentImageInfor->lstAllRectangles.begin() + numberOfRectangles, currentImageInfor->lstAllRectangles.end());
		}
		else
		{
			// Cập nhật thông tin hình chữ nhật vào dataGridView
			int currentColumn = 0;
			int currentRow = 0;
			if (this->dataGridView1->CurrentCell != nullptr)
			{
				currentColumn = this->dataGridView1->CurrentCell->ColumnIndex;
				currentRow = this->dataGridView1->CurrentCell->RowIndex;
			}
			this->dataGridView1->Rows->Clear();
			for (int i = 0; i < numberOfRectangles; i++)
			{
				cv::Rect rectangle = currentImageInfor->lstAllRectangles[i];
				System::String^ position = L"(" + rectangle.x.ToString() + "," + rectangle.y.ToString() + ")";
				System::String^ size = L"(" + rectangle.width.ToString() + "," + rectangle.height.ToString() + ")";
				this->dataGridView1->Rows->Add(i, position, size);
			}
			this->dataGridView1->Rows[currentRow]->Cells[currentColumn]->Selected = true;
		}
	}

	// Cập nhật trạng thái nút và các controls khác
	btnEdiable->Text = m_isEdiable ? L"NoEdiable" : L"Ediable";

	btnSave->Enabled = true;
	btnNext->Enabled = true;
	btnPrevious->Enabled = true;
	tbNoAllPoles->Enabled = true;
	btnEdiable->Enabled = true;
	btnReset->Enabled = true;
	strIndex->Text = L"" + (m_imageIdx + 1).ToString() + "/" + m_totalImage.ToString();

	// Hiển thị hình ảnh cập nhật
	this->pictureBox->Image = m_image;

#ifdef _DEBUG_
	cv::Mat src = cv::Mat(m_image->Height, m_image->Width, CV_8UC3);
	Bitmap^ bmp = gcnew Bitmap(m_image);
	BitmapConverter::ToMat(bmp, src);
	bmp->~Bitmap();
	cv::Mat dst;
	cvtColor(src, dst, cv::COLOR_RGB2GRAY);
#endif // _DEBUG
}

void XavisTech::MainForm::Reset()
{
	m_totalImage = 0;
	m_isMouseDown = false;
	m_imageIdx = 0;
	m_scale = 0;
	m_offsetPoint = System::Drawing::Point(0, 0);
	btnSave->Enabled = false;
	btnEdiable->Enabled = false;
	btnNext->Enabled = false;
	tbNoAllPoles->Enabled = false;
	btnPrevious->Enabled = false;
	this->dataGridView1->Rows->Clear();
	strIndex->Text = L"000/000";
	strXY->Text = L"(0:0)";
	fnSystem = nullptr;
	lstImageInfo.clear();
	btnReset->Enabled = true;
	if(m_image != nullptr)
		m_image->~Image();
}

System::Void XavisTech::MainForm::pictureBox_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	if (m_totalImage <= 0) return;

	if (m_isMouseDown && e->Button == System::Windows::Forms::MouseButtons::Left)  // Kiểm tra chuột trái đang được giữ
	{
		// Tạo vùng hình chữ nhật từ điểm bắt đầu tới điểm hiện tại
		m_rectCurrent = System::Drawing::Rectangle(
			Math::Min(m_rectStartPoint.X, e->Location.X),
			Math::Min(m_rectStartPoint.Y, e->Location.Y),
			Math::Abs(m_rectStartPoint.X - e->Location.X),
			Math::Abs(m_rectStartPoint.Y - e->Location.Y)
		);

		// Vẽ lại hình chữ nhật
		pictureBox->Invalidate();  // Yêu cầu vẽ lại ảnh
	}
}

System::Void XavisTech::MainForm::pictureBox_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	m_isMouseDown = false;
	if (m_totalImage <= 0) return;

	m_isMouseDown = false;
	if (m_totalImage <= 0) return;

	return;
}

System::Void XavisTech::MainForm::pictureBox_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e)
{
	// Nếu có hình chữ nhật đang vẽ, hiển thị nó
	if (m_isMouseDown)
	{
		System::Drawing::Pen^ rectPen = gcnew System::Drawing::Pen(Color::Red, 2);  // Pen màu đỏ để vẽ hình chữ nhật
		e->Graphics->DrawRectangle(rectPen, m_rectCurrent);  // Vẽ hình chữ nhật tạm thời
	}
}

System::Void XavisTech::MainForm::btnEditable_Click(System::Object^ sender, System::EventArgs^ e)
{
	if (m_totalImage <= 0) return;
	m_isEdiable = !m_isEdiable;

	Update();
}

System::Void XavisTech::MainForm::textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e)
{
	Reset();
	if (System::IO::Directory::Exists(textBox1->Text))
	{
		array<System::String^>^ searchPattern = { "*.BMP","*.TIF","*.JPG" };
		int lenght = 0;
		for each (System::String ^ var in searchPattern)
		{
			array<System::String^>^ lstFile = System::IO::Directory::GetFiles(textBox1->Text, var, System::IO::SearchOption::TopDirectoryOnly);
			if (fnSystem != nullptr)
				lenght = fnSystem->Length;

			System::Array::Resize(fnSystem, lenght + lstFile->Length);
			lstFile->CopyTo(fnSystem, lenght);
		}
		if (lenght > 0)
			m_totalImage = fnSystem->Length;
	}
	if (m_totalImage > 0)
	{
		m_imageIdx = 0;
		lstImageInfo.resize(m_totalImage);
		for each (ImageInfo imgInfo in lstImageInfo)
		{
			imgInfo.noAllPoles = Convert::ToInt32(tbNoAllPoles->Text);
		}
		m_image = System::Drawing::Bitmap::FromFile(fnSystem[m_imageIdx]);
		caculateScaleOffset();
	}

	Update();
}

System::Void XavisTech::MainForm::btnNext_Click(System::Object^ sender, System::EventArgs^ e)
{
	if (m_imageIdx < m_totalImage - 1)
	{
		m_image->~Image();
		m_imageIdx++;
		strXY->Text = L"(0:0)";
		m_image = System::Drawing::Bitmap::FromFile(fnSystem[m_imageIdx]);
		caculateScaleOffset();
		tbNoAllPoles->Text = L"" + lstImageInfo[m_imageIdx].noAllPoles;
		this->dataGridView1->Rows->Clear();
		cellEdit = System::Drawing::Point(0, 0);
		Update();
	}
}

System::Void XavisTech::MainForm::btnPrevious_Click(System::Object^ sender, System::EventArgs^ e)
{
	if (m_imageIdx > 0)
	{
		m_image->~Image();
		m_imageIdx--;
		strXY->Text = L"(0:0)";
		m_image = System::Drawing::Bitmap::FromFile(fnSystem[m_imageIdx]);
		caculateScaleOffset();
		tbNoAllPoles->Text = L"" + lstImageInfo[m_imageIdx].noAllPoles;
		this->dataGridView1->Rows->Clear();
		cellEdit = System::Drawing::Point(0, 0);
		Update();
	}
}

System::Void XavisTech::MainForm::btnBrown1_Click(System::Object^ sender, System::EventArgs^ e)
{
	System::Windows::Forms::FolderBrowserDialog^ folderBrowserDialog = gcnew System::Windows::Forms::FolderBrowserDialog();
	if (folderBrowserDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		this->textBox1->Text = folderBrowserDialog->SelectedPath;
	}
}

void DrawPoint(cv::Mat& src, std::tuple<int, cv::Point, cv::Point> var, AlternateColor alterColor, Shape poleShape)
{
	cv::Scalar lightBluePen(255,255,51);
	cv::Scalar bluePen(255,0,0);
	cv::Scalar cathodePen, anodePen;
	int thickness = 1;
	int radio = 3;
	int widthLine = 4;
	switch (alterColor)
	{
	case AlternateColor::UP_DOWN:
		cathodePen = bluePen;
		anodePen = lightBluePen;
		break;
	case AlternateColor::LEFT_RIGHT:
		if (std::get<0>(var) % 2 == 0)
		{
			cathodePen = bluePen;
			anodePen = bluePen;
		}
		else
		{
			cathodePen = lightBluePen;
			anodePen = lightBluePen;
		}
		break;
	default:
		cathodePen = bluePen;
		anodePen = bluePen;
		break;
	}

	if (std::get<1>(var).x > 0)
	{
		switch (poleShape)
		{
		case Shape::LINE:
		{
			cv::Point p1(std::get<1>(var).x - widthLine, std::get<1>(var).y);
			cv::Point p2(std::get<1>(var).x + widthLine, std::get<1>(var).y);
			//e->Graphics->DrawLine(cathodePen, p1, p2);
			cv::line(src, p1, p2, cathodePen, thickness);
		}
		break;
		case Shape::CIRCLE:
		{
			cv::Point p1 = std::get<1>(var);
			cv::circle(src, p1, radio, cathodePen, thickness);
		}
		break;
		case Shape::POINT:
		default:
			cv::Point p1 = std::get<1>(var);
			cv::circle(src, p1, 2, cathodePen, -thickness);
			break;
		}
	}

	if (std::get<2>(var).x > 0)
	{
		switch (poleShape)
		{
		case Shape::LINE:
		{
			cv::Point p1(std::get<2>(var).x - widthLine, std::get<2>(var).y);
			cv::Point p2(std::get<2>(var).x + widthLine, std::get<2>(var).y);
			cv::line(src, p1, p2, anodePen, thickness);
			//e->Graphics->DrawLine(anodePen, p1, p2);
		}
		break;
		case Shape::CIRCLE:
		{
			cv::Point p1 = std::get<2>(var);
			cv::circle(src, p1, radio, anodePen, thickness);
		}
		break;
		case Shape::POINT:
		default:
			cv::Point p1 = std::get<2>(var);
			cv::circle(src, p1, 2, anodePen, -thickness);
			break;
		}
	}
}

System::Void XavisTech::MainForm::btnSave_Click(System::Object^ sender, System::EventArgs^ e)
{
	if (m_totalImage <= 0) return;

	for (int i = 0; i < m_totalImage; i++) {
		std::wstring imgfile = (const wchar_t*)(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(fnSystem[i])).ToPointer();
		/*std::size_t found = imgfile.find_last_of(L"/\\");
		std::wstring currentImage = imgfile.substr(found + 1);*/
		std::wstring strFolderName = imgfile.substr(0, imgfile.find_last_of(L"/\\") + 1);
		//std::string strFolderNameSave = strFilePath.substr(0, strFilePath.find_last_of("/\\") + 1) + "data_input\\";
		std::wstring strFileName = imgfile.substr(imgfile.find_last_of(L"/\\") + 1);
		std::wstring strFileNameOnly = strFileName.substr(0, strFileName.find_last_of(L"."));
		ImageInfo* currentImageInfor = &lstImageInfo[i];

		Image^ img = System::Drawing::Bitmap::FromFile(fnSystem[i]);
		cv::Size imgSize(img->Width, img->Height);
		cv::Mat markImg = cv::Mat::zeros(imgSize, CV_8UC3);
		std::wofstream ofs;
		ofs.open(strFolderName + strFileNameOnly + L".txt", std::ofstream::out);
		ofs << "Index\t" << "Cathode\t\t" << "Anode" << "\n";
		for each (std::tuple<int, cv::Point, cv::Point> var in currentImageInfor->lstAllPolesPosition)
		{
			if (std::get<1>(var).x > 0 && (std::get<2>(var).x > 0))
			{
				DrawPoint(markImg, var, m_alternateColor, m_poleShape);
				ofs << std::get<0>(var) << "\t" << "(" << std::get<1>(var).x << "," << std::get<1>(var).y << ")"
					<< "\t" << "(" << std::get<2>(var).x << "," << std::get<2>(var).y << ")" << "\n";// << L"(" + std::get<1>(var).x.ToString() + "," + std::get<1>(var).y.ToString() + ")";
			}
		}
		ofs.close();
		Bitmap^ bmp1 = BitmapConverter::ToBitmap(markImg);
		bmp1->Save(fnSystem[i] + "_" + static_cast<int>(m_poleShape).ToString() + ".jpg");
		img->~Image();
		markImg.release();
		bmp1->~Bitmap();
	}

	MessageBox::Show(this,"Finish save result!", "Save Result",MessageBoxButtons::OK);

	//System::Windows::Forms::SaveFileDialog^ saveFileDialog = gcnew System::Windows::Forms::SaveFileDialog();
	//saveFileDialog->Filter = "Text file (*.txt)|*.txt";
	//saveFileDialog->Title = "Save a text file";
	//if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	//{
	//	std::wstring filename = (const wchar_t*)(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(saveFileDialog->FileName)).ToPointer();
	//	//open file for writing
	//	std::wofstream fw(filename, std::ofstream::out);

	//	//check if file was successfully opened for writing
	//	if (fw.is_open())
	//	{
	//		fw << "Index\t" << "Cathode\t" << "Anode" << "\n";
	//		//store array contents to text file
	//		
	//		//close the file after the writing operation is completed
	//		fw.close();
	//	}
	//}
}

System::Void XavisTech::MainForm::pictureBox_MouseWheel(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
{
	if (m_totalImage <= 0) return;

	if (e->X >= pictureBox->Location.X && e->X <= pictureBox->Location.X + pictureBox->Width
		&& e->Y >= pictureBox->Location.Y && e->Y <= pictureBox->Location.Y + pictureBox->Height)
	{
		//  flag = 1;
		// Override OnMouseWheel event, for zooming in/out with the scroll wheel
		if (pictureBox->Image != nullptr)
		{
			// If the mouse wheel is moved backward (Zoom in)
			if (e->Delta > 0)
			{
				zoomScale += zoomStep;
				if (zoomScale > 10) zoomScale = 10;

			}
			else //Zoom out
			{
				zoomScale -= zoomStep;
				if (zoomScale < 0.1) zoomScale = 0.1;
			}

			int old_w = pictureBox->Width;
			int old_h = pictureBox->Height;
			pictureBox->Width = m_picBoxOrgSize.Width * zoomScale;
			pictureBox->Height = m_picBoxOrgSize.Height * zoomScale;

			// Change the Scroll
			if (zoomScale > 1) {
				vScrollBar1->Enabled = true;
				hScrollBar1->Enabled = true;
				/*vScrollBar1->Maximum = ceil(zoomScale) * 19;
				vScrollBar1->LargeChange = MAX(floor(vScrollBar1->Maximum / 10), 1);
				hScrollBar1->Maximum = ceil(zoomScale) * 10;
				hScrollBar1->LargeChange = MAX(floor(hScrollBar1->Maximum / 10), 1);*/

				vScrollBar1->Maximum = pictureBox->Height - panel3->Height + m_offsetPoint.Y * 2;
				vScrollBar1->LargeChange = MAX(floor(vScrollBar1->Maximum / 20), 1);
				hScrollBar1->Maximum = pictureBox->Width - panel3->Width + m_offsetPoint.X * 2;
				hScrollBar1->LargeChange = MAX(floor(hScrollBar1->Maximum / 20), 1);

				//int x = e->X - panel3->Left;
				//int y = e->Y - panel3->Top;
				int x = e->X;
				int y = e->Y;
				int x_new = x + (pictureBox->Left - x) * ((float)pictureBox->Width / old_w);
				int y_new = y + (pictureBox->Top - y) * ((float)pictureBox->Height / old_h);

				/*vScrollBar1->Value = MIN(MAX((float)abs(panel3->Top - y_new) / (pictureBox->Height - panel3->Height) * vScrollBar1->Maximum, vScrollBar1->Minimum), vScrollBar1->Maximum);
				hScrollBar1->Value = MIN(MAX((float)abs(panel3->Left - x_new) / (pictureBox->Width - panel3->Width) * hScrollBar1->Maximum, hScrollBar1->Minimum), hScrollBar1->Maximum);*/

				vScrollBar1->Value = MIN(MAX(panel3->Top + m_offsetPoint.Y - y_new, vScrollBar1->Minimum), vScrollBar1->Maximum);
				hScrollBar1->Value = MIN(MAX(panel3->Left + m_offsetPoint.X - x_new, hScrollBar1->Minimum), hScrollBar1->Maximum);
			}
			else {
				vScrollBar1->Enabled = false;
				hScrollBar1->Enabled = false;
				/*vScrollBar1->Maximum = 0;
				vScrollBar1->LargeChange = 1;
				hScrollBar1->Maximum = 0;
				hScrollBar1->LargeChange = 1;*/

				pictureBox->Top = m_centerPanel.Y - pictureBox->Height / 2;
				pictureBox->Left = m_centerPanel.X - pictureBox->Width / 2;
			}
		}

		m_scale = (float)pictureBox->Width / m_image->Width;
		Update();
	}

	this->OnMouseWheel(e);
}

System::Void XavisTech::MainForm::btnOpenFile_Click(System::Object^ sender, System::EventArgs^ e)
{
	System::Windows::Forms::OpenFileDialog^ openFile = gcnew System::Windows::Forms::OpenFileDialog();
	openFile->Filter = "(*.TIF)|*.TIF|(*.BMP)|*.BMP|(*.JPG)|*.JPG";

	// Allow the user to select multiple images.
	openFile->Multiselect = true;
	openFile->Title = "My Image Browser";
	System::Windows::Forms::DialogResult dialogResult =  openFile->ShowDialog();

	if (dialogResult == System::Windows::Forms::DialogResult::OK)
	{
		Reset();
		m_totalImage = openFile->FileNames->Length;
		fnSystem = openFile->FileNames;
	}

	if (m_totalImage > 0)
	{
		m_imageIdx = 0;
		lstImageInfo.resize(m_totalImage);
		for (auto it = lstImageInfo.begin(); it < lstImageInfo.end(); it++)
		{
			it->noAllPoles = Convert::ToInt32(tbNoAllPoles->Text);
		}
		m_image = System::Drawing::Bitmap::FromFile(fnSystem[m_imageIdx]);
		caculateScaleOffset();
	}

	Update();
}

System::Void XavisTech::MainForm::vScrollBar1_ValueChanged(System::Object^ sender, System::EventArgs^ e)
{
	if (m_totalImage <= 0) return;

	int minPos = panel3->Top + panel3->Height - pictureBox->Height - m_offsetPoint.Y * 2;

	int diffH = panel3->Top - (float)vScrollBar1->Value;
	
	pictureBox->Top = MAX(MIN(diffH, panel3->Top) + m_offsetPoint.Y, minPos);
}

System::Void XavisTech::MainForm::hScrollBar1_ValueChanged(System::Object^ sender, System::EventArgs^ e)
{
	if (m_totalImage <= 0) return;

	int minPos = panel3->Left + panel3->Width - pictureBox->Width - m_offsetPoint.X * 2;

	int diffH = panel3->Left - (float)hScrollBar1->Value;

	pictureBox->Left = MAX(MIN(diffH, panel3->Left) + m_offsetPoint.X, minPos);
}

System::Void XavisTech::MainForm::dataGridView1_CellMouseClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellMouseEventArgs^ e)
{
	switch (e->ColumnIndex)
	{
	case 0:
		cellEdit = System::Drawing::Point(0,0);
		break;
	case 1:
	case 2:
		cellEdit = m_isEdiable ? System::Drawing::Point(e->ColumnIndex, e->RowIndex) : System::Drawing::Point(0, 0);
		break;
	default:
		cellEdit = System::Drawing::Point(0, 0);
		break;
	}
	pictureBox->Refresh();
}

System::Void XavisTech::MainForm::tbNoAllPoles_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e)
{
	if (!Char::IsControl(e->KeyChar) && !Char::IsDigit(e->KeyChar) &&
		(e->KeyChar != '.'))
	{
		e->Handled = true;
	}
}

System::Void XavisTech::MainForm::tbNoAllPoles_Validated(System::Object^ sender, System::EventArgs^ e)
{
	ImageInfo* currentImageInfor = &lstImageInfo[m_imageIdx];

	currentImageInfor->noAllPoles = Convert::ToInt32(tbNoAllPoles->Text);
	Update();
}

System::Void XavisTech::MainForm::radioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
{
	m_alternateColor = rbAlternate_updown->Checked ? AlternateColor::UP_DOWN : AlternateColor::LEFT_RIGHT;
	if (rbShape_point->Checked) m_poleShape = Shape::POINT;
	else if(rbShape_line->Checked)  m_poleShape = Shape::LINE;
	else  m_poleShape = Shape::CIRCLE;

	pictureBox->Refresh();
}

System::Void XavisTech::MainForm::btnReset_Click(System::Object^ sender, System::EventArgs^ e)
{
	ImageInfo* currentImageInfor = &lstImageInfo[m_imageIdx];
	int numberOfPoles = currentImageInfor->noAllPoles;
	currentImageInfor->lstAllPolesPosition.clear();
	for (int i = 0; i < numberOfPoles; i++)
	{
		currentImageInfor->lstAllPolesPosition.push_back(std::make_tuple(i, cv::Point(-1, -1), cv::Point(-1, -1)));
	}
	Update();
}

void XavisTech::MainForm::InitializeComponent(void)
{
	System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle1 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
	System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle2 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
	this->pictureBox = (gcnew System::Windows::Forms::PictureBox());
	this->panel1 = (gcnew System::Windows::Forms::Panel());
	this->strIndex = (gcnew System::Windows::Forms::Label());
	this->label7 = (gcnew System::Windows::Forms::Label());
	this->textBox1 = (gcnew System::Windows::Forms::TextBox());
	this->label5 = (gcnew System::Windows::Forms::Label());
	this->btnBrown1 = (gcnew System::Windows::Forms::Button());
	this->btnPrevious = (gcnew System::Windows::Forms::Button());
	this->btnNext = (gcnew System::Windows::Forms::Button());
	this->btnOpenFile = (gcnew System::Windows::Forms::Button());
	this->btnSave = (gcnew System::Windows::Forms::Button());
	this->btnEdiable = (gcnew System::Windows::Forms::Button());
	this->strXY = (gcnew System::Windows::Forms::Label());
	this->label2 = (gcnew System::Windows::Forms::Label());
	this->panel2 = (gcnew System::Windows::Forms::Panel());
	this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
	this->rbShape_point = (gcnew System::Windows::Forms::RadioButton());
	this->rbShape_circle = (gcnew System::Windows::Forms::RadioButton());
	this->rbShape_line = (gcnew System::Windows::Forms::RadioButton());
	this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
	this->rbAlternate_leftright = (gcnew System::Windows::Forms::RadioButton());
	this->rbAlternate_updown = (gcnew System::Windows::Forms::RadioButton());
	this->tbNoAllPoles = (gcnew System::Windows::Forms::TextBox());
	this->label1 = (gcnew System::Windows::Forms::Label());
	this->label4 = (gcnew System::Windows::Forms::Label());
	this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
	this->btnReset = (gcnew System::Windows::Forms::Button());
	this->panel3 = (gcnew System::Windows::Forms::Panel());
	this->vScrollBar1 = (gcnew System::Windows::Forms::VScrollBar());
	this->hScrollBar1 = (gcnew System::Windows::Forms::HScrollBar());
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox))->BeginInit();
	this->panel1->SuspendLayout();
	this->panel2->SuspendLayout();
	this->groupBox2->SuspendLayout();
	this->groupBox1->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->BeginInit();
	this->panel3->SuspendLayout();
	this->SuspendLayout();
	// 
	// pictureBox
	// 
	this->pictureBox->BackColor = System::Drawing::Color::White;
	this->pictureBox->Location = System::Drawing::Point(0, 0);
	this->pictureBox->Name = L"pictureBox";
	this->pictureBox->Size = System::Drawing::Size(828, 740);
	this->pictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
	this->pictureBox->TabIndex = 1;
	this->pictureBox->TabStop = false;
	this->pictureBox->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::pictureBox_Paint);
	this->pictureBox->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox_MouseDown);
	this->pictureBox->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox_MouseMove);
	this->pictureBox->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox_MouseUp);
	// 
	// panel1
	// 
	this->panel1->Controls->Add(this->strIndex);
	this->panel1->Controls->Add(this->label7);
	this->panel1->Controls->Add(this->textBox1);
	this->panel1->Controls->Add(this->label5);
	this->panel1->Controls->Add(this->btnBrown1);
	this->panel1->Controls->Add(this->btnPrevious);
	this->panel1->Controls->Add(this->btnNext);
	this->panel1->Controls->Add(this->btnOpenFile);
	this->panel1->Dock = System::Windows::Forms::DockStyle::Bottom;
	this->panel1->Location = System::Drawing::Point(0, 761);
	this->panel1->Name = L"panel1";
	this->panel1->Size = System::Drawing::Size(1185, 59);
	this->panel1->TabIndex = 2;
	// 
	// strIndex
	// 
	this->strIndex->AutoSize = true;
	this->strIndex->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->strIndex->Location = System::Drawing::Point(752, 21);
	this->strIndex->Name = L"strIndex";
	this->strIndex->Size = System::Drawing::Size(67, 17);
	this->strIndex->TabIndex = 19;
	this->strIndex->Text = L"000/000";
	// 
	// label7
	// 
	this->label7->AutoSize = true;
	this->label7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->label7->Location = System::Drawing::Point(700, 20);
	this->label7->Name = L"label7";
	this->label7->Size = System::Drawing::Size(46, 18);
	this->label7->TabIndex = 19;
	this->label7->Text = L"Index:";
	// 
	// textBox1
	// 
	this->textBox1->Location = System::Drawing::Point(162, 19);
	this->textBox1->Name = L"textBox1";
	this->textBox1->Size = System::Drawing::Size(416, 20);
	this->textBox1->TabIndex = 5;
	this->textBox1->TextChanged += gcnew System::EventHandler(this, &MainForm::textBox1_TextChanged);
	// 
	// label5
	// 
	this->label5->AutoSize = true;
	this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->label5->Location = System::Drawing::Point(12, 21);
	this->label5->Name = L"label5";
	this->label5->Size = System::Drawing::Size(135, 17);
	this->label5->TabIndex = 4;
	this->label5->Text = L"Choose Source File:";
	// 
	// btnBrown1
	// 
	this->btnBrown1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnBrown1->Location = System::Drawing::Point(591, 18);
	this->btnBrown1->Name = L"btnBrown1";
	this->btnBrown1->Size = System::Drawing::Size(75, 23);
	this->btnBrown1->TabIndex = 6;
	this->btnBrown1->Text = L"Browse...";
	this->btnBrown1->UseVisualStyleBackColor = true;
	this->btnBrown1->Click += gcnew System::EventHandler(this, &MainForm::btnBrown1_Click);
	// 
	// btnPrevious
	// 
	this->btnPrevious->Enabled = false;
	this->btnPrevious->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnPrevious->Location = System::Drawing::Point(966, 12);
	this->btnPrevious->Name = L"btnPrevious";
	this->btnPrevious->Size = System::Drawing::Size(90, 36);
	this->btnPrevious->TabIndex = 0;
	this->btnPrevious->Text = L"Previous";
	this->btnPrevious->UseVisualStyleBackColor = true;
	this->btnPrevious->Click += gcnew System::EventHandler(this, &MainForm::btnPrevious_Click);
	// 
	// btnNext
	// 
	this->btnNext->Enabled = false;
	this->btnNext->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnNext->Location = System::Drawing::Point(1075, 12);
	this->btnNext->Name = L"btnNext";
	this->btnNext->Size = System::Drawing::Size(90, 36);
	this->btnNext->TabIndex = 0;
	this->btnNext->Text = L"Next";
	this->btnNext->UseVisualStyleBackColor = true;
	this->btnNext->Click += gcnew System::EventHandler(this, &MainForm::btnNext_Click);
	// 
	// btnOpenFile
	// 
	this->btnOpenFile->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnOpenFile->Location = System::Drawing::Point(857, 12);
	this->btnOpenFile->Name = L"btnOpenFile";
	this->btnOpenFile->Size = System::Drawing::Size(90, 36);
	this->btnOpenFile->TabIndex = 0;
	this->btnOpenFile->Text = L"Open File";
	this->btnOpenFile->UseVisualStyleBackColor = true;
	this->btnOpenFile->Click += gcnew System::EventHandler(this, &MainForm::btnOpenFile_Click);
	// 
	// btnSave
	// 
	this->btnSave->Enabled = false;
	this->btnSave->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnSave->Location = System::Drawing::Point(225, 722);
	this->btnSave->Name = L"btnSave";
	this->btnSave->Size = System::Drawing::Size(90, 36);
	this->btnSave->TabIndex = 0;
	this->btnSave->Text = L"Save";
	this->btnSave->UseVisualStyleBackColor = true;
	this->btnSave->Click += gcnew System::EventHandler(this, &MainForm::btnSave_Click);
	// 
	// btnEdiable
	// 
	this->btnEdiable->Enabled = false;
	this->btnEdiable->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnEdiable->Location = System::Drawing::Point(7, 722);
	this->btnEdiable->Name = L"btnEdiable";
	this->btnEdiable->Size = System::Drawing::Size(90, 36);
	this->btnEdiable->TabIndex = 0;
	this->btnEdiable->Text = L"Ediable";
	this->btnEdiable->UseVisualStyleBackColor = true;
	this->btnEdiable->Click += gcnew System::EventHandler(this, &MainForm::btnEditable_Click);
	// 
	// strXY
	// 
	this->strXY->AutoSize = true;
	this->strXY->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->strXY->Location = System::Drawing::Point(62, 708);
	this->strXY->Name = L"strXY";
	this->strXY->Size = System::Drawing::Size(43, 17);
	this->strXY->TabIndex = 2;
	this->strXY->Text = L"(0:0)";
	// 
	// label2
	// 
	this->label2->AutoSize = true;
	this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->label2->Location = System::Drawing::Point(18, 708);
	this->label2->Name = L"label2";
	this->label2->Size = System::Drawing::Size(45, 17);
	this->label2->TabIndex = 2;
	this->label2->Text = L"(x,y):";
	// 
	// panel2
	// 
	this->panel2->Controls->Add(this->groupBox2);
	this->panel2->Controls->Add(this->groupBox1);
	this->panel2->Controls->Add(this->tbNoAllPoles);
	this->panel2->Controls->Add(this->label1);
	this->panel2->Controls->Add(this->label4);
	this->panel2->Controls->Add(this->dataGridView1);
	this->panel2->Controls->Add(this->btnSave);
	this->panel2->Controls->Add(this->btnReset);
	this->panel2->Controls->Add(this->btnEdiable);
	this->panel2->Dock = System::Windows::Forms::DockStyle::Right;
	this->panel2->Location = System::Drawing::Point(850, 0);
	this->panel2->Name = L"panel2";
	this->panel2->Size = System::Drawing::Size(335, 761);
	this->panel2->TabIndex = 4;
	// 
	// groupBox2
	// 
	this->groupBox2->Controls->Add(this->rbShape_point);
	this->groupBox2->Controls->Add(this->rbShape_circle);
	this->groupBox2->Controls->Add(this->rbShape_line);
	this->groupBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->groupBox2->Location = System::Drawing::Point(15, 647);
	this->groupBox2->Name = L"groupBox2";
	this->groupBox2->Size = System::Drawing::Size(300, 61);
	this->groupBox2->TabIndex = 25;
	this->groupBox2->TabStop = false;
	this->groupBox2->Text = L"Shape";
	// 
	// rbShape_point
	// 
	this->rbShape_point->AutoSize = true;
	this->rbShape_point->Checked = true;
	this->rbShape_point->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->rbShape_point->Location = System::Drawing::Point(117, 22);
	this->rbShape_point->Name = L"rbShape_point";
	this->rbShape_point->Size = System::Drawing::Size(58, 21);
	this->rbShape_point->TabIndex = 25;
	this->rbShape_point->TabStop = true;
	this->rbShape_point->Text = L"Point";
	this->rbShape_point->UseVisualStyleBackColor = true;
	this->rbShape_point->CheckedChanged += gcnew System::EventHandler(this, &MainForm::radioButton_CheckedChanged);
	// 
	// rbShape_circle
	// 
	this->rbShape_circle->AutoSize = true;
	this->rbShape_circle->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->rbShape_circle->Location = System::Drawing::Point(203, 22);
	this->rbShape_circle->Name = L"rbShape_circle";
	this->rbShape_circle->Size = System::Drawing::Size(61, 21);
	this->rbShape_circle->TabIndex = 24;
	this->rbShape_circle->Text = L"Circle";
	this->rbShape_circle->UseVisualStyleBackColor = true;
	this->rbShape_circle->CheckedChanged += gcnew System::EventHandler(this, &MainForm::radioButton_CheckedChanged);
	// 
	// rbShape_line
	// 
	this->rbShape_line->AutoSize = true;
	this->rbShape_line->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->rbShape_line->Location = System::Drawing::Point(27, 22);
	this->rbShape_line->Name = L"rbShape_line";
	this->rbShape_line->Size = System::Drawing::Size(53, 21);
	this->rbShape_line->TabIndex = 23;
	this->rbShape_line->Text = L"Line";
	this->rbShape_line->UseVisualStyleBackColor = true;
	this->rbShape_line->CheckedChanged += gcnew System::EventHandler(this, &MainForm::radioButton_CheckedChanged);
	// 
	// groupBox1
	// 
	this->groupBox1->Controls->Add(this->rbAlternate_leftright);
	this->groupBox1->Controls->Add(this->rbAlternate_updown);
	this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->groupBox1->Location = System::Drawing::Point(15, 580);
	this->groupBox1->Name = L"groupBox1";
	this->groupBox1->Size = System::Drawing::Size(300, 61);
	this->groupBox1->TabIndex = 25;
	this->groupBox1->TabStop = false;
	this->groupBox1->Text = L"Alternate Color";
	// 
	// rbAlternate_leftright
	// 
	this->rbAlternate_leftright->AutoSize = true;
	this->rbAlternate_leftright->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular,
		System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
	this->rbAlternate_leftright->Location = System::Drawing::Point(176, 22);
	this->rbAlternate_leftright->Name = L"rbAlternate_leftright";
	this->rbAlternate_leftright->Size = System::Drawing::Size(88, 21);
	this->rbAlternate_leftright->TabIndex = 21;
	this->rbAlternate_leftright->Text = L"Left-Right";
	this->rbAlternate_leftright->UseVisualStyleBackColor = true;
	this->rbAlternate_leftright->CheckedChanged += gcnew System::EventHandler(this, &MainForm::radioButton_CheckedChanged);
	// 
	// rbAlternate_updown
	// 
	this->rbAlternate_updown->AutoSize = true;
	this->rbAlternate_updown->Checked = true;
	this->rbAlternate_updown->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->rbAlternate_updown->Location = System::Drawing::Point(24, 22);
	this->rbAlternate_updown->Name = L"rbAlternate_updown";
	this->rbAlternate_updown->Size = System::Drawing::Size(84, 21);
	this->rbAlternate_updown->TabIndex = 21;
	this->rbAlternate_updown->TabStop = true;
	this->rbAlternate_updown->Text = L"Up-Down";
	this->rbAlternate_updown->UseVisualStyleBackColor = true;
	this->rbAlternate_updown->CheckedChanged += gcnew System::EventHandler(this, &MainForm::radioButton_CheckedChanged);
	// 
	// tbNoAllPoles
	// 
	this->tbNoAllPoles->Enabled = false;
	this->tbNoAllPoles->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->tbNoAllPoles->Location = System::Drawing::Point(191, 497);
	this->tbNoAllPoles->MaxLength = 2;
	this->tbNoAllPoles->Name = L"tbNoAllPoles";
	this->tbNoAllPoles->Size = System::Drawing::Size(88, 24);
	this->tbNoAllPoles->TabIndex = 20;
	this->tbNoAllPoles->Text = L"10";
	this->tbNoAllPoles->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
	this->tbNoAllPoles->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainForm::tbNoAllPoles_KeyPress);
	this->tbNoAllPoles->Validated += gcnew System::EventHandler(this, &MainForm::tbNoAllPoles_Validated);
	// 
	// label1
	// 
	this->label1->AutoSize = true;
	this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->label1->Location = System::Drawing::Point(12, 543);
	this->label1->Name = L"label1";
	this->label1->Size = System::Drawing::Size(100, 17);
	this->label1->TabIndex = 11;
	this->label1->Text = L"Result Setting:";
	// 
	// label4
	// 
	this->label4->AutoSize = true;
	this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->label4->Location = System::Drawing::Point(12, 500);
	this->label4->Name = L"label4";
	this->label4->Size = System::Drawing::Size(124, 18);
	this->label4->TabIndex = 19;
	this->label4->Text = L"Number of Poles:";
	// 
	// dataGridView1
	// 
	this->dataGridView1->AllowUserToDeleteRows = false;
	this->dataGridView1->AllowUserToResizeColumns = false;
	this->dataGridView1->AllowUserToResizeRows = false;
	dataGridViewCellStyle1->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
	dataGridViewCellStyle1->BackColor = System::Drawing::SystemColors::Control;
	dataGridViewCellStyle1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	dataGridViewCellStyle1->ForeColor = System::Drawing::SystemColors::WindowText;
	dataGridViewCellStyle1->SelectionBackColor = System::Drawing::SystemColors::Highlight;
	dataGridViewCellStyle1->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
	dataGridViewCellStyle1->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
	this->dataGridView1->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
	this->dataGridView1->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
	this->dataGridView1->Location = System::Drawing::Point(15, 12);
	this->dataGridView1->MultiSelect = false;
	this->dataGridView1->Name = L"dataGridView1";
	this->dataGridView1->RowHeadersVisible = false;
	this->dataGridView1->RowHeadersWidth = 50;
	dataGridViewCellStyle2->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
	dataGridViewCellStyle2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->dataGridView1->RowsDefaultCellStyle = dataGridViewCellStyle2;
	this->dataGridView1->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
	this->dataGridView1->Size = System::Drawing::Size(300, 456);
	this->dataGridView1->TabIndex = 10;
	this->dataGridView1->CellContentClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::dataGridView1_CellContentClick);
	this->dataGridView1->CellMouseClick += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &MainForm::dataGridView1_CellMouseClick);
	// 
	// btnReset
	// 
	this->btnReset->Enabled = false;
	this->btnReset->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.5F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->btnReset->Location = System::Drawing::Point(116, 722);
	this->btnReset->Name = L"btnReset";
	this->btnReset->Size = System::Drawing::Size(90, 36);
	this->btnReset->TabIndex = 0;
	this->btnReset->Text = L"Reset";
	this->btnReset->UseVisualStyleBackColor = true;
	this->btnReset->Click += gcnew System::EventHandler(this, &MainForm::btnReset_Click);
	// 
	// panel3
	// 
	this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(64)), static_cast<System::Int32>(static_cast<System::Byte>(64)),
		static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->panel3->Controls->Add(this->pictureBox);
	this->panel3->Location = System::Drawing::Point(0, 0);
	this->panel3->Name = L"panel3";
	this->panel3->Size = System::Drawing::Size(828, 740);
	this->panel3->TabIndex = 5;
	// 
	// vScrollBar1
	// 
	this->vScrollBar1->Enabled = false;
	this->vScrollBar1->LargeChange = 20;
	this->vScrollBar1->Location = System::Drawing::Point(833, 2);
	this->vScrollBar1->Name = L"vScrollBar1";
	this->vScrollBar1->Size = System::Drawing::Size(17, 738);
	this->vScrollBar1->TabIndex = 2;
	this->vScrollBar1->Value = 20;
	this->vScrollBar1->ValueChanged += gcnew System::EventHandler(this, &MainForm::vScrollBar1_ValueChanged);
	// 
	// hScrollBar1
	// 
	this->hScrollBar1->Enabled = false;
	this->hScrollBar1->Location = System::Drawing::Point(0, 743);
	this->hScrollBar1->Name = L"hScrollBar1";
	this->hScrollBar1->Size = System::Drawing::Size(832, 17);
	this->hScrollBar1->TabIndex = 6;
	this->hScrollBar1->ValueChanged += gcnew System::EventHandler(this, &MainForm::hScrollBar1_ValueChanged);
	// 
	// MainForm
	// 
	this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
	this->ClientSize = System::Drawing::Size(1185, 820);
	this->Controls->Add(this->hScrollBar1);
	this->Controls->Add(this->vScrollBar1);
	this->Controls->Add(this->panel2);
	this->Controls->Add(this->strXY);
	this->Controls->Add(this->label2);
	this->Controls->Add(this->panel3);
	this->Controls->Add(this->panel1);
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
	this->MaximizeBox = false;
	this->MinimizeBox = false;
	this->Name = L"MainForm";
	this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
	this->Text = L" ";
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox))->EndInit();
	this->panel1->ResumeLayout(false);
	this->panel1->PerformLayout();
	this->panel2->ResumeLayout(false);
	this->panel2->PerformLayout();
	this->groupBox2->ResumeLayout(false);
	this->groupBox2->PerformLayout();
	this->groupBox1->ResumeLayout(false);
	this->groupBox1->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->EndInit();
	this->panel3->ResumeLayout(false);
	this->ResumeLayout(false);
	this->PerformLayout();

}

