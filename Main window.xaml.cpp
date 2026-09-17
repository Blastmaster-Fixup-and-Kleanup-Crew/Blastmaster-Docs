#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Graphics.Printing.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Text;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

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

    // --- File Menu Item Commands ---

    void MainWindow::New_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().SetText(TextSetOptions::None, L"");
        m_currentFile = nullptr;
        Editor().IsEnabled(true);
    }

    IAsyncAction MainWindow::Open_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileOpenPicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeFilter().Append(L".rtf");
        picker.FileTypeFilter().Append(L".txt");

        StorageFile file = co_await picker.PickSingleFileAsync();
        if (file != nullptr)
        {
            streams::IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::Read);
            Editor().Document().LoadFromStream(TextSetOptions::FormatRtf, stream);
            m_currentFile = file;
            Editor().IsEnabled(true);
            AddToRecentFiles(file.Path());
        }
    }

    void MainWindow::Close_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Editor().Document().SetText(TextSetOptions::None, L"");
        m_currentFile = nullptr;
        Editor().IsEnabled(false); // Disables editing area while document is closed
    }

    IAsyncAction MainWindow::Save_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_currentFile == nullptr)
        {
            co_await SaveAs_Click(nullptr, nullptr);
        }
        else
        {
            streams::IRandomAccessStream stream = co_await m_currentFile.OpenAsync(FileAccessMode::ReadWrite);
            Editor().Document().SaveToStream(TextGetOptions::FormatRtf, stream);
        }
    }

    IAsyncAction MainWindow::SaveAs_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileSavePicker picker;
        picker.as<IInitializeWithWindow>()->Initialize(GetWindowHandle());
        picker.FileTypeChoices().Insert(L"Rich Text Format", winrt::single_threaded_vector<hstring>({ L".rtf" }));
        picker.FileTypeChoices().Insert(L"Plain Text", winrt::single_threaded_vector<hstring>({ L".txt" }));
        picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
        picker.SuggestedFileName(L"Document");

        StorageFile file = co_await picker.PickSaveFileAsync();
        if (file != nullptr)
        {
            streams::IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::ReadWrite);
            Editor().Document().SaveToStream(TextGetOptions::FormatRtf, stream);
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
        paperSize.Items().Append(box_value(L"Legal (8.5 x 14 in)"));
        paperSize.SelectedIndex(0);

        TextBox margins;
        margins.Header(box_value(L"Margins (inches)"));
        margins.Text(L"1.0");

        panel.Children().Append(paperSize);
        panel.Children().Append(margins);

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
        // Triggers the system print manager UI contract
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
        
        TextBox authorText;
        authorText.Header(box_value(L"Author"));
        authorText.Text(L"WinUI User");

        panel.Children().Append(locationText);
        panel.Children().Append(statsText);
        panel.Children().Append(authorText);

        dialog.Content(panel);
        dialog.CloseButtonText(L"Close");

        co_await dialog.ShowAsync();
    }

    IAsyncAction MainWindow::Send_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Opens the default mail provider with the document content pre-filled
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

    // --- Recent Files Logic ---

    void MainWindow::AddToRecentFiles(hstring const& filePath)
    {
        // Deduplicate
        auto it = std::find(m_recentFiles.begin(), m_recentFiles.end(), filePath);
        if (it != m_recentFiles.end())
        {
            m_recentFiles.erase(it);
        }

        // Insert at head
        m_recentFiles.insert(m_recentFiles.begin(), filePath);

        // Cap size at 4
        if (m_recentFiles.size() > 4)
        {
            m_recentFiles.pop_back();
        }

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
            streams::IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::Read);
            Editor().Document().LoadFromStream(TextSetOptions::FormatRtf, stream);
            m_currentFile = file;
            Editor().IsEnabled(true);
        }
        catch (...)
        {
            // File may have been moved or deleted
        }
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
