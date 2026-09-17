#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Graphics.Printing.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Text;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

namespace winrt::WordProcessorApp::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    HWND MainWindow::GetWindowHandle()
    {
        auto windowNative = m_inner.as<::IWindowNative>();
        HWND hwnd{ nullptr };
        winrt::check_hresult(windowNative->get_WindowHandle(&hwnd));
        return hwnd;
    }

    // --- XML Serialization and Deserialization (.dccx / .dctx) ---

    IAsyncAction MainWindow::SaveCustomXmlFile(StorageFile const& file, bool isTemplate)
    {
        // 1. Extract Rich Text payload
        InMemoryRandomAccessStream memoryStream;
        Editor().Document().SaveToStream(TextGetOptions::FormatRtf, memoryStream);
        
        DataReader reader(memoryStream.GetInputStreamAt(0));
        co_await reader.LoadAsync(static_cast<uint32_t>(memoryStream.Size()));
        hstring rtfContent = reader.ReadString(reader.UnconsumedBufferLength());

        // 2. Build XML DOM Structure
        XmlDocument doc;
        hstring rootTag = isTemplate ? L"DocumentTemplate" : L"Document";
        hstring rootNs = isTemplate ? L"http://schemas.dccx.org/2026/template" : L"http://schemas.dccx.org/2026/document";

        XmlElement root = doc.CreateElement(rootTag);
        root.SetAttribute(L"xmlns", rootNs);
        root.SetAttribute(L"Version", L"1.0");
        doc.AppendChild(root);

        // Metadata Properties
        XmlElement props = doc.CreateElement(L"Properties");
        XmlElement title = doc.CreateElement(L"Title");
        title.InnerText(file.DisplayName());
        props.AppendChild(title);
        root.AppendChild(props);

        // Page Layout
        XmlElement pageSetup = doc.CreateElement(L"PageSetup");
        pageSetup.SetAttribute(L"Size", L"Letter");
        pageSetup.SetAttribute(L"Orientation", L"Portrait");
        root.AppendChild(pageSetup);

        // Body Content encapsulating RTF CDATA
        XmlElement body = doc.CreateElement(isTemplate ? L"InitialContent" : L"Body");
        XmlElement rtfNode = doc.CreateElement(L"RtfContent");
        
        // Append raw RTF text within node
        rtfNode.InnerText(rtfContent);
        body.AppendChild(rtfNode);
        root.AppendChild(body);

        // 3. Write XML output stream
        co_await FileIO::WriteTextAsync(file, doc.GetXml());
    }

    IAsyncAction MainWindow::LoadCustomXmlFile(StorageFile const& file, bool isTemplateInit)
    {
        hstring xmlText = co_await FileIO::ReadTextAsync(file);
        XmlDocument doc;
        doc.LoadXml(xmlText);

        XmlNodeList rtfNodes = doc.GetElementsByTagName(L"RtfContent");
        if (rtfNodes.Length() > 0)
        {
            hstring rtfContent = rtfNodes.Item(0).InnerText();

            InMemoryRandomAccessStream memoryStream;
            DataWriter writer(memoryStream);
            writer.WriteString(rtfContent);
            co_await writer.StoreAsync();
            co_await writer.FlushAsync();
            memoryStream.Seek(0);

            Editor().Document().LoadFromStream(TextSetOptions::FormatRtf, memoryStream);
        }

        if (!isTemplateInit)
        {
            m_currentFile = file;
            AddToRecentFiles(file.Path());
        }
        else
        {
            // If initialized from a template, treat as an unsaved new document
            m_currentFile = nullptr;
        }

        Editor().IsEnabled(true);
    }

    // --- File Menu Handlers ---

    void MainWindow::New_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().SetText(TextSetOptions::None, L"");
        m_currentFile = nullptr;
        Editor().IsEnabled(true);
    }

    IAsyncAction MainWindow::NewFromTemplate_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileOpenPicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeFilter().Append(L".dctx");

        StorageFile file = co_await picker.PickSingleFileAsync();
        if (file != nullptr)
        {
            co_await LoadCustomXmlFile(file, true);
        }
    }

    IAsyncAction MainWindow::Open_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileOpenPicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeFilter().Append(L".dccx");
        picker.FileTypeFilter().Append(L".dctx");
        picker.FileTypeFilter().Append(L".rtf");
        picker.FileTypeFilter().Append(L".txt");

        StorageFile file = co_await picker.PickSingleFileAsync();
        if (file != nullptr)
        {
            if (file.FileType() == L".dccx" || file.FileType() == L".dctx")
            {
                co_await LoadCustomXmlFile(file, false);
            }
            else
            {
                IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::Read);
                Editor().Document().LoadFromStream(TextSetOptions::FormatRtf, stream);
                m_currentFile = file;
                Editor().IsEnabled(true);
                AddToRecentFiles(file.Path());
            }
        }
    }

    void MainWindow::Close_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().SetText(TextSetOptions::None, L"");
        m_currentFile = nullptr;
        Editor().IsEnabled(false);
    }

    IAsyncAction MainWindow::Save_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_currentFile == nullptr)
        {
            co_await SaveAs_Click(nullptr, nullptr);
        }
        else if (m_currentFile.FileType() == L".dccx" || m_currentFile.FileType() == L".dctx")
        {
            co_await SaveCustomXmlFile(m_currentFile, m_currentFile.FileType() == L".dctx");
        }
        else
        {
            IRandomAccessStream stream = co_await m_currentFile.OpenAsync(FileAccessMode::ReadWrite);
            Editor().Document().SaveToStream(TextGetOptions::FormatRtf, stream);
        }
    }

    IAsyncAction MainWindow::SaveAs_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileSavePicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeChoices().Insert(L"Custom XML Document (*.dccx)", winrt::single_threaded_vector<hstring>({ L".dccx" }));
        picker.FileTypeChoices().Insert(L"Rich Text Format (*.rtf)", winrt::single_threaded_vector<hstring>({ L".rtf" }));
        picker.FileTypeChoices().Insert(L"Plain Text (*.txt)", winrt::single_threaded_vector<hstring>({ L".txt" }));
        picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
        picker.SuggestedFileName(L"Document");

        StorageFile file = co_await picker.PickSaveFileAsync();
        if (file != nullptr)
        {
            if (file.FileType() == L".dccx")
            {
                co_await SaveCustomXmlFile(file, false);
            }
            else
            {
                IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::ReadWrite);
                Editor().Document().SaveToStream(TextGetOptions::FormatRtf, stream);
            }
            m_currentFile = file;
            AddToRecentFiles(file.Path());
        }
    }

    IAsyncAction MainWindow::SaveAsTemplate_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileSavePicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeChoices().Insert(L"Custom Document Template (*.dctx)", winrt::single_threaded_vector<hstring>({ L".dctx" }));
        picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
        picker.SuggestedFileName(L"Template");

        StorageFile file = co_await picker.PickSaveFileAsync();
        if (file != nullptr)
        {
            co_await SaveCustomXmlFile(file, true);
            m_currentFile = file;
            AddToRecentFiles(file.Path());
        }
    }

    IAsyncAction MainWindow::PageSetup_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Page Setup"));

        StackPanel panel;
        panel.Spacing(10);

        ComboBox paperSize;
        paperSize.Header(box_value(L"Paper Size"));
        paperSize.Items().Append(box_value(L"Letter (8.5 x 11 in)"));
        paperSize.Items().Append(box_value(L"A4 (210 x 297 mm)"));
        paperSize.SelectedIndex(0);

        panel.Children().Append(paperSize);
        dialog.Content(panel);
        dialog.CloseButtonText(L"OK");

        co_await dialog.ShowAsync();
    }

    IAsyncAction MainWindow::PrintPreview_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Print Preview"));

        TextBlock previewText;
        hstring contentText;
        Editor().Document().GetText(TextGetOptions::None, contentText);
        previewText.Text(L"--- Page 1 Preview ---\n\n" + contentText);
        previewText.TextWrapping(TextWrapping::Wrap);

        ScrollViewer viewer;
        viewer.Content(previewText);
        viewer.Height(300);

        dialog.Content(viewer);
        dialog.CloseButtonText(L"Close");

        co_await dialog.ShowAsync();
    }

    IAsyncAction MainWindow::Print_Click(IInspectable const&, RoutedEventArgs const&)
    {
        co_await Windows::Graphics::Printing::PrintManager::ShowPrintUIAsync();
    }

    IAsyncAction MainWindow::Properties_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Document Properties"));

        hstring docText;
        Editor().Document().GetText(TextGetOptions::None, docText);
        
        uint32_t charCount = docText.size();
        uint32_t wordCount = 0;
        bool inWord = false;
        for (wchar_t c : docText)
        {
            if (iswspace(c)) inWord = false;
            else if (!inWord) { inWord = true; wordCount++; }
        }

        hstring fileLocation = m_currentFile != nullptr ? m_currentFile.Path() : L"Unsaved Document";

        StackPanel panel;
        panel.Spacing(8);

        TextBlock locationText;
        locationText.Text(L"Path: " + fileLocation);
        TextBlock statsText;
        statsText.Text(L"Words: " + to_hstring(wordCount) + L"  |  Characters: " + to_hstring(charCount));

        panel.Children().Append(locationText);
        panel.Children().Append(statsText);

        dialog.Content(panel);
        dialog.CloseButtonText(L"Close");

        co_await dialog.ShowAsync();
    }

    IAsyncAction MainWindow::Send_Click(IInspectable const&, RoutedEventArgs const&)
    {
        hstring bodyText;
        Editor().Document().GetText(TextGetOptions::None, bodyText);

        hstring mailToUri = L"mailto:?subject=" + Uri::EscapeComponent(L"Document Shared") +
                            L"&body=" + Uri::EscapeComponent(bodyText);

        co_await Windows::System::Launcher::LaunchUriAsync(Uri(mailToUri));
    }

    void MainWindow::Exit_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Application::Current().Exit();
    }

    // --- Recent Files Management ---

    void MainWindow::AddToRecentFiles(hstring const& filePath)
    {
        auto it = std::find(m_recentFiles.begin(), m_recentFiles.end(), filePath);
        if (it != m_recentFiles.end()) m_recentFiles.erase(it);

        m_recentFiles.insert(m_recentFiles.begin(), filePath);
        if (m_recentFiles.size() > 4) m_recentFiles.pop_back();

        RefreshRecentFilesMenu();
    }

    void MainWindow::RefreshRecentFilesMenu()
    {
        MenuFlyoutSubItem menu = RecentFilesMenu();
        menu.Items().Clear();

        for (hstring const& path : m_recentFiles)
        {
            MenuFlyoutItem item;
            item.Text(path);
            item.Click([this, path](IInspectable const&, RoutedEventArgs const&) {
                OpenFileFromPath(path);
            });
            menu.Items().Append(item);
        }
    }

    IAsyncAction MainWindow::OpenFileFromPath(hstring const& filePath)
    {
        try
        {
            StorageFile file = co_await StorageFile::GetFileFromPathAsync(filePath);
            if (file.FileType() == L".dccx" || file.FileType() == L".dctx")
            {
                co_await LoadCustomXmlFile(file, false);
            }
            else
            {
                IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::Read);
                Editor().Document().LoadFromStream(TextSetOptions::FormatRtf, stream);
                m_currentFile = file;
                Editor().IsEnabled(true);
            }
        }
        catch (...) {}
    }

    // --- Formatting Logic ---

    void MainWindow::BoldButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextSelection selection = Editor().Document().Selection();
        if (selection != nullptr)
        {
            ITextCharacterFormat format = selection.CharacterFormat();
            format.Bold(format.Bold() == FormatEffect::On ? FormatEffect::Off : FormatEffect::On);
            selection.CharacterFormat(format);
        }
    }

    void MainWindow::ItalicButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextSelection selection = Editor().Document().Selection();
        if (selection != nullptr)
        {
            ITextCharacterFormat format = selection.CharacterFormat();
            format.Italic(format.Italic() == FormatEffect::On ? FormatEffect::Off : FormatEffect::On);
            selection.CharacterFormat(format);
        }
    }

    void MainWindow::UnderlineButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextSelection selection = Editor().Document().Selection();
        if (selection != nullptr)
        {
            ITextCharacterFormat format = selection.CharacterFormat();
            format.Underline(format.Underline() == UnderlineType::Single ? UnderlineType::None : UnderlineType::Single);
            selection.CharacterFormat(format);
        }
    }

    void MainWindow::FontSizeIncrease_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextSelection selection = Editor().Document().Selection();
        if (selection != nullptr)
        {
            ITextCharacterFormat format = selection.CharacterFormat();
            format.Size(format.Size() + 2.0f);
            selection.CharacterFormat(format);
        }
    }

    void MainWindow::FontSizeDecrease_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextSelection selection = Editor().Document().Selection();
        if (selection != nullptr && selection.CharacterFormat().Size() > 4.0f)
        {
            ITextCharacterFormat format = selection.CharacterFormat();
            format.Size(format.Size() - 2.0f);
            selection.CharacterFormat(format);
        }
    }
}
