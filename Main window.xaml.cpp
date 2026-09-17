#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
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
using namespace Windows::ApplicationModel::DataTransfer;
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

    // --- Edit Menu Implementations ---

    void MainWindow::Undo_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (Editor().Document().CanUndo())
        {
            Editor().Document().Undo();
        }
    }

    void MainWindow::Redo_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (Editor().Document().CanRedo())
        {
            Editor().Document().Redo();
        }
    }

    void MainWindow::Cut_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().Selection().Cut();
    }

    void MainWindow::Copy_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().Selection().Copy();
    }

    void MainWindow::Paste_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().Selection().Paste(0);
    }

    IAsyncAction MainWindow::PasteSpecial_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Paste Special"));

        StackPanel panel;
        panel.Spacing(10);

        RadioButton rtfOption;
        rtfOption.Content(box_value(L"Formatted Text (RTF)"));
        rtfOption.IsChecked(true);

        RadioButton unformattedOption;
        unformattedOption.Content(box_value(L"Unformatted Text"));

        RadioButton unicodeOption;
        unicodeOption.Content(box_value(L"Unicode Text"));

        panel.Children().Append(rtfOption);
        panel.Children().Append(unformattedOption);
        panel.Children().Append(unicodeOption);

        dialog.Content(panel);
        dialog.PrimaryButtonText(L"Paste");
        dialog.CloseButtonText(L"Cancel");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary)
        {
            if (unformattedOption.IsChecked().Value())
            {
                // Retrieve unformatted plain text from Clipboard
                DataPackageView dataPackage = Clipboard::GetContent();
                if (dataPackage.Contains(StandardDataFormats::Text()))
                {
                    hstring text = co_await dataPackage.GetTextAsync();
                    Editor().Document().Selection().SetText(TextSetOptions::None, text);
                }
            }
            else
            {
                Editor().Document().Selection().Paste(0);
            }
        }
    }

    void MainWindow::Clear_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().Selection().SetText(TextSetOptions::None, L"");
    }

    void MainWindow::SelectAll_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ITextRange range = Editor().Document().GetRange(0, c_hkTextMax);
        Editor().Document().Selection().SetRange(0, c_hkTextMax);
    }

    IAsyncAction MainWindow::Find_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Find"));

        TextBox findInput;
        findInput.Header(box_value(L"Find what:"));
        findInput.PlaceholderText(L"Enter search text...");

        dialog.Content(findInput);
        dialog.PrimaryButtonText(L"Find Next");
        dialog.CloseButtonText(L"Cancel");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary && !findInput.Text().empty())
        {
            ITextRange range = Editor().Document().GetRange(0, 0);
            int32_t found = range.FindText(findInput.Text(), c_hkTextMax, FindOptions::None);
            if (found > 0)
            {
                Editor().Document().Selection().SetRange(range.StartPosition(), range.EndPosition());
            }
        }
    }

    IAsyncAction MainWindow::Replace_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Find and Replace"));

        StackPanel panel;
        panel.Spacing(10);

        TextBox findInput;
        findInput.Header(box_value(L"Find what:"));

        TextBox replaceInput;
        replaceInput.Header(box_value(L"Replace with:"));

        panel.Children().Append(findInput);
        panel.Children().Append(replaceInput);

        dialog.Content(panel);
        dialog.PrimaryButtonText(L"Replace All");
        dialog.CloseButtonText(L"Cancel");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary && !findInput.Text().empty())
        {
            ITextRange range = Editor().Document().GetRange(0, c_hkTextMax);
            hstring text;
            range.GetText(TextGetOptions::None, text);

            std::wstring str(text.c_str());
            std::wstring from(findInput.Text().c_str());
            std::wstring to(replaceInput.Text().c_str());

            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::wstring::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }

            Editor().Document().SetText(TextSetOptions::None, hstring(str));
        }
    }

    IAsyncAction MainWindow::GoTo_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Go To"));

        TextBox lineInput;
        lineInput.Header(box_value(L"Enter line number:"));
        lineInput.Text(L"1");

        dialog.Content(lineInput);
        dialog.PrimaryButtonText(L"Go To");
        dialog.CloseButtonText(L"Cancel");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary)
        {
            int lineNum = std::wcstol(lineInput.Text().c_str(), nullptr, 10);
            ITextRange range = Editor().Document().GetRange(0, 0);
            range.Move(TextRangeUnit::Line, lineNum - 1);
            Editor().Document().Selection().SetRange(range.StartPosition(), range.StartPosition());
        }
    }

    IAsyncAction MainWindow::AutoText_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"AutoText Entries"));

        ListView listView;
        listView.Items().Append(box_value(L"Sincerely,"));
        listView.Items().Append(box_value(L"Thank you for your business."));
        listView.Items().Append(box_value(L"Confidential Document"));
        listView.SelectedIndex(0);

        dialog.Content(listView);
        dialog.PrimaryButtonText(L"Insert");
        dialog.CloseButtonText(L"Cancel");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary && listView.SelectedItem() != nullptr)
        {
            hstring autoText = unbox_value<hstring>(listView.SelectedItem());
            Editor().Document().Selection().SetText(TextSetOptions::None, autoText);
        }
    }

    IAsyncAction MainWindow::Bookmark_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Bookmarks"));

        StackPanel panel;
        panel.Spacing(10);

        TextBox nameInput;
        nameInput.Header(box_value(L"Bookmark Name:"));

        ListView listView;
        for (auto const& [name, pos] : m_bookmarks)
        {
            listView.Items().Append(box_value(name));
        }

        panel.Children().Append(nameInput);
        panel.Children().Append(listView);

        dialog.Content(panel);
        dialog.PrimaryButtonText(L"Add");
        dialog.SecondaryButtonText(L"Go To");
        dialog.CloseButtonText(L"Close");

        ContentDialogResult result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary && !nameInput.Text().empty())
        {
            int32_t pos = Editor().Document().Selection().StartPosition();
            m_bookmarks[nameInput.Text()] = pos;
        }
        else if (result == ContentDialogResult::Secondary && listView.SelectedItem() != nullptr)
        {
            hstring selectedName = unbox_value<hstring>(listView.SelectedItem());
            if (m_bookmarks.find(selectedName) != m_bookmarks.end())
            {
                int32_t pos = m_bookmarks[selectedName];
                Editor().Document().Selection().SetRange(pos, pos);
            }
        }
    }

    IAsyncAction MainWindow::Links_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Edit Links"));

        TextBlock desc;
        desc.Text(L"Manage linked external objects and dynamic files.");

        ListView listView;
        listView.Items().Append(box_value(L"C:\\Data\\ChartData.xlsx (Excel.Sheet) - Automatic Update"));

        StackPanel panel;
        panel.Spacing(10);
        panel.Children().Append(desc);
        panel.Children().Append(listView);

        dialog.Content(panel);
        dialog.PrimaryButtonText(L"Update Now");
        dialog.CloseButtonText(L"Close");

        co_await dialog.ShowAsync();
    }

    IAsyncAction MainWindow::Object_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(Content().XamlRoot());
        dialog.Title(box_value(L"Insert Object"));

        ListBox typeList;
        typeList.Items().Append(box_value(L"Bitmap Image"));
        typeList.Items().Append(box_value(L"Graph Chart"));
        typeList.Items().Append(box_value(L"PDF Document"));
        typeList.SelectedIndex(0);

        dialog.Content(typeList);
        dialog.PrimaryButtonText(L"Insert");
        dialog.CloseButtonText(L"Cancel");

        co_await dialog.ShowAsync();
    }

    // --- XML Serialization and Deserialization (.dccx / .dctx) ---

    IAsyncAction MainWindow::SaveCustomXmlFile(StorageFile const& file, bool isTemplate)
    {
        InMemoryRandomAccessStream memoryStream;
        Editor().Document().SaveToStream(TextGetOptions::FormatRtf, memoryStream);

        DataReader reader(memoryStream.GetInputStreamAt(0));
        co_await reader.LoadAsync(static_cast<uint32_t>(memoryStream.Size()));
        hstring rtfContent = reader.ReadString(reader.UnconsumedBufferLength());

        XmlDocument doc;
        hstring rootTag = isTemplate ? L"DocumentTemplate" : L"Document";
        hstring rootNs = isTemplate ? L"http://schemas.dccx.org/2026/template" : L"http://schemas.dccx.org/2026/document";

        XmlElement root = doc.CreateElement(rootTag);
        root.SetAttribute(L"xmlns", rootNs);
        root.SetAttribute(L"Version", L"1.0");
        doc.AppendChild(root);

        XmlElement props = doc.CreateElement(L"Properties");
        XmlElement title = doc.CreateElement(L"Title");
        title.InnerText(file.DisplayName());
        props.AppendChild(title);
        root.AppendChild(props);

        XmlElement body = doc.CreateElement(isTemplate ? L"InitialContent" : L"Body");
        XmlElement rtfNode = doc.CreateElement(L"RtfContent");
        rtfNode.InnerText(rtfContent);
        body.AppendChild(rtfNode);
        root.AppendChild(body);

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
            m_currentFile = nullptr;
        }

        Editor().IsEnabled(true);
    }

    // --- File Menu Commands ---

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
