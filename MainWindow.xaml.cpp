#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::WinUI3TextEditor::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        Title(L"WinUI 3 Text Editor");
    }

    void MainWindow::OnFileItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        if (action == L"Exit")
        {
            Close();
        }
        else if (action == L"New")
        {
            EditorBox().Text(L"");
            StatusText().Text(L"Created new document.");
        }
        else
        {
            StatusText().Text(L"File Action: " + action);
        }
    }

    void MainWindow::OnEditItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        if (action == L"Undo" && EditorBox().CanUndo())
        {
            EditorBox().Undo();
        }
        else if (action == L"Redo" && EditorBox().CanRedo())
        {
            EditorBox().Redo();
        }
        else if (action == L"Cut")
        {
            EditorBox().CutSelectionToClipboard();
        }
        else if (action == L"Copy")
        {
            EditorBox().CopySelectionToClipboard();
        }
        else if (action == L"Paste")
        {
            EditorBox().PasteFromClipboard();
        }
        else if (action == L"Select All")
        {
            EditorBox().SelectAll();
        }

        StatusText().Text(L"Edit Action: " + action);
    }

    void MainWindow::OnViewModeClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring mode = item.Text();

        StatusText().Text(L"Mode: " + mode + L" | Zoom: 100%");
    }

    void MainWindow::OnToggleToolbarClick(IInspectable const&, RoutedEventArgs const&)
    {
        bool isStandardOn = StandardToolbarToggle().IsChecked();
        bool isFormattingOn = FormattingToolbarToggle().IsChecked();

        if (isStandardOn || isFormattingOn)
        {
            ToolbarsPanel().Visibility(Visibility::Visible);
        }
        else
        {
            ToolbarsPanel().Visibility(Visibility::Collapsed);
        }
    }

    void MainWindow::OnToggleRulerClick(IInspectable const&, RoutedEventArgs const&)
    {
        if (RulerToggle().IsChecked())
        {
            RulerBar().Visibility(Visibility::Visible);
        }
        else
        {
            RulerBar().Visibility(Visibility::Collapsed);
        }
    }

    void MainWindow::OnFullScreenToggle(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto toggleItem = sender.as<ToggleMenuFlyoutItem>();
        bool isFullScreen = toggleItem.IsChecked();

        if (isFullScreen)
        {
            StatusText().Text(L"Full Screen Mode Active");
        }
        else
        {
            StatusText().Text(L"Mode: Normal | Zoom: 100%");
        }
    }

    void MainWindow::OnOpenPaneClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring paneName = item.Text();

        StatusText().Text(L"Opened Pane: " + paneName);
    }

    void MainWindow::OnZoomClick(IInspectable const&, RoutedEventArgs const&)
    {
        StatusText().Text(L"Zoom Dialog Requested");
    }

    void MainWindow::OnInsertItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Inserted: " + action);
    }

    void MainWindow::OnFormatItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Format Option Selected: " + action);
    }

    void MainWindow::OnToolsItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Tools Action: " + action);
    }

    void MainWindow::OnTableItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Table Action: " + action);
    }

    void MainWindow::OnToggleGridlinesClick(IInspectable const&, RoutedEventArgs const&)
    {
        bool showGridlines = GridlinesToggle().IsChecked();
        if (showGridlines)
        {
            StatusText().Text(L"Table Gridlines: Visible");
        }
        else
        {
            StatusText().Text(L"Table Gridlines: Hidden");
        }
    }

    void MainWindow::OnWindowItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Window Action: " + action);
    }

    void MainWindow::OnSwitchDocumentClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<RadioMenuFlyoutItem>();
        hstring docName = item.Text();

        StatusText().Text(L"Switched to Document: " + docName);
    }
}
