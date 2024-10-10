#pragma once
#include <vector>
#include <string>

enum class AlternateColor : int
{
	UP_DOWN,
	LEFT_RIGHT
};

enum class Shape : int
{
	LINE,
	CIRCLE,
	POINT
};

namespace XavisTech {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	//using namespace Microsoft::WindowsAPICodePack::Dialogs;
	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
		

	public:
		MainForm(void);
	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm();
	private: System::Windows::Forms::PictureBox^ pictureBox;
	protected:
	private: System::Windows::Forms::Panel^ panel1;
	private: System::Windows::Forms::Label^ strXY;
	private: System::Windows::Forms::Label^ label2;
	private: System::Windows::Forms::Button^ btnSave;


	private: System::Windows::Forms::Button^ btnPrevious;
	private: System::Windows::Forms::Button^ btnNext;
	private: System::Windows::Forms::Button^ btnEdiable;
	private: System::Windows::Forms::Label^ strIndex;
	private: System::Windows::Forms::Panel^ panel2;
	private: System::Windows::Forms::Panel^ panel3;
	private: System::Windows::Forms::DataGridView^ dataGridView1;
	private: System::Windows::Forms::Label^ label7;
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::Button^ btnBrown1;
	private: System::Windows::Forms::VScrollBar^ vScrollBar1;
	private: System::Windows::Forms::HScrollBar^ hScrollBar1;
	private: System::Windows::Forms::Label^ label1;
	private: System::Windows::Forms::Button^ btnReset;
	private: System::Windows::Forms::TextBox^ tbNoAllPoles;
	private: System::Windows::Forms::Label^ label4;
	private: System::Windows::Forms::Button^ btnOpenFile;
	private: System::Windows::Forms::RadioButton^ rbShape_line;
	private: System::Windows::Forms::RadioButton^ rbAlternate_leftright;
	private: System::Windows::Forms::RadioButton^ rbAlternate_updown;
	private: System::Windows::Forms::RadioButton^ radioButton3;
	private: System::Windows::Forms::RadioButton^ rbShape_point;
	private: System::Windows::Forms::RadioButton^ rbShape_circle;
	private: System::Windows::Forms::RadioButton^ radioButton1;
	private: System::Windows::Forms::GroupBox^ groupBox2;
	private: System::Windows::Forms::GroupBox^ groupBox1;
	private: System::Windows::Forms::RadioButton^ radioButton5;
	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
	private:
		System::Drawing::Image^		m_image;
		array<System::String^>^		fnSystem;
		System::Drawing::Point		m_centerPanel;
		System::Drawing::Point		m_offsetPoint;
		System::Drawing::Rectangle	m_picBoxOrgSize;
		AlternateColor		m_alternateColor;
		Shape				m_poleShape;
		bool				m_isMouseDown;
		float				m_scale;
		bool				m_isEdiable;
		int					m_imageIdx;
		int					m_totalImage;
		System::Drawing::Point m_rectStartPoint;  // ?i?m b?t ??u v? hình ch? nh?t
		System::Drawing::Rectangle m_rectCurrent;  // Hình ch? nh?t hi?n t?i
#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void);
		void Update();
		void Reset();
		void caculateScaleOffset();
#pragma endregion
	private: System::Void pictureBox_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
	private: System::Void pictureBox_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
	private: System::Void pictureBox_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
	private: System::Void pictureBox_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
	private: System::Void pictureBox_MouseWheel(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);
	private: System::Void btnEditable_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e);
	private: System::Void btnNext_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void btnPrevious_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void btnBrown1_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void btnSave_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void btnOpenFile_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void vScrollBar1_ValueChanged(System::Object^ sender, System::EventArgs^ e);
	private: System::Void hScrollBar1_ValueChanged(System::Object^ sender, System::EventArgs^ e);
	private: System::Void dataGridView1_CellMouseClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellMouseEventArgs^ e);
	private: System::Void tbNoAllPoles_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e);
	private: System::Void tbNoAllPoles_Validated(System::Object^ sender, System::EventArgs^ e);
	private: System::Void radioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e);
private: System::Void btnReset_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void dataGridView1_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e) {
	}
	};
}
