#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Controls::Primitives;

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
        bool isBordersOn = BordersToolbarToggle().IsChecked();
        bool isDrawingOn = DrawingToolbarToggle().IsChecked();
        bool isFormsOn = FormsToolbarToggle().IsChecked();

        StandardToolbar().Visibility(isStandardOn ? Visibility::Visible : Visibility::Collapsed);
        FormattingToolbar().Visibility(isFormattingOn ? Visibility::Visible : Visibility::Collapsed);
        BordersToolbar().Visibility(isBordersOn ? Visibility::Visible : Visibility::Collapsed);
        DrawingToolbar().Visibility(isDrawingOn ? Visibility::Visible : Visibility::Collapsed);
        FormsSidebar().Visibility(isFormsOn ? Visibility::Visible : Visibility::Collapsed);

        DrawingToolbarButton().IsChecked(isDrawingOn);
        BordersToolbarButton().IsChecked(isBordersOn);
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

    void MainWindow::OnHelpItemClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        hstring action = item.Text();

        StatusText().Text(L"Help Action: " + action);
    }

    void MainWindow::OnStandardToolbarButtonClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = L"";

        if (auto btn = sender.try_as<Button>())
        {
            tag = unbox_value<hstring>(btn.Tag());
        }
        else if (auto toggleBtn = sender.try_as<ToggleButton>())
        {
            tag = unbox_value<hstring>(toggleBtn.Tag());
        }

        if (tag == L"Drawing")
        {
            bool showDrawing = DrawingToolbarButton().IsChecked().Value();
            DrawingToolbar().Visibility(showDrawing ? Visibility::Visible : Visibility::Collapsed);
            DrawingToolbarToggle().IsChecked(showDrawing);
            StatusText().Text(showDrawing ? L"Drawing Toolbar Activated" : L"Drawing Toolbar Deactivated");
        }
        else if (tag == L"New")
        {
            EditorBox().Text(L"");
            StatusText().Text(L"Created new document.");
        }
        else if (tag == L"Undo" && EditorBox().CanUndo())
        {
            EditorBox().Undo();
            StatusText().Text(L"Standard Toolbar Action: Undo");
        }
        else if (tag == L"Redo" && EditorBox().CanRedo())
        {
            EditorBox().Redo();
            StatusText().Text(L"Standard Toolbar Action: Redo");
        }
        else if (tag == L"Cut")
        {
            EditorBox().CutSelectionToClipboard();
            StatusText().Text(L"Standard Toolbar Action: Cut");
        }
        else if (tag == L"Copy")
        {
            EditorBox().CopySelectionToClipboard();
            StatusText().Text(L"Standard Toolbar Action: Copy");
        }
        else if (tag == L"Paste")
        {
            EditorBox().PasteFromClipboard();
            StatusText().Text(L"Standard Toolbar Action: Paste");
        }
        else
        {
            StatusText().Text(L"Standard Toolbar Action: " + tag);
        }
    }

    void MainWindow::OnZoomComboBoxChanged(IInspectable const& sender, SelectionChangedEventArgs const&)
    {
        if (auto combo = sender.try_as<ComboBox>())
        {
            if (auto selectedItem = combo.SelectedItem().try_as<ComboBoxItem>())
            {
                hstring val = unbox_value<hstring>(selectedItem.Content());
                StatusText().Text(L"Zoom level set to: " + val);
            }
        }
    }

    void MainWindow::OnDrawingToolbarButtonClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = L"";

        if (auto btn = sender.try_as<Button>())
        {
            tag = unbox_value<hstring>(btn.Tag());
        }
        else if (auto toggleBtn = sender.try_as<ToggleButton>())
        {
            tag = unbox_value<hstring>(toggleBtn.Tag());
        }
        else if (auto item = sender.try_as<MenuFlyoutItem>())
        {
            tag = unbox_value<hstring>(item.Tag());
        }

        StatusText().Text(L"Drawing Tool Selected: " + tag);
    }

    void MainWindow::OnFormattingToolbarButtonClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = L"";

        if (auto btn = sender.try_as<Button>())
        {
            tag = unbox_value<hstring>(btn.Tag());
        }
        else if (auto toggleBtn = sender.try_as<ToggleButton>())
        {
            tag = unbox_value<hstring>(toggleBtn.Tag());
        }
        else if (auto item = sender.try_as<MenuFlyoutItem>())
        {
            tag = unbox_value<hstring>(item.Tag());
        }

        if (tag == L"Borders Toolbar")
        {
            bool showBorders = BordersToolbarButton().IsChecked().Value();
            BordersToolbar().Visibility(showBorders ? Visibility::Visible : Visibility::Collapsed);
            BordersToolbarToggle().IsChecked(showBorders);
            StatusText().Text(showBorders ? L"Borders Toolbar Activated" : L"Borders Toolbar Deactivated");
        }
        else
        {
            StatusText().Text(L"Formatting Command Selected: " + tag);
        }
    }

    void MainWindow::OnFormattingComboBoxChanged(IInspectable const& sender, SelectionChangedEventArgs const&)
    {
        if (auto combo = sender.try_as<ComboBox>())
        {
            hstring category = unbox_value<hstring>(combo.Tag());
            if (auto selectedItem = combo.SelectedItem().try_as<ComboBoxItem>())
            {
                hstring val = unbox_value<hstring>(selectedItem.Content());
                StatusText().Text(L"Formatting " + category + L" set to: " + val);
            }
        }
    }

    void MainWindow::OnFormsToolbarButtonClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = L"";

        if (auto btn = sender.try_as<Button>())
        {
            tag = unbox_value<hstring>(btn.Tag());
        }
        else if (auto toggleBtn = sender.try_as<ToggleButton>())
        {
            tag = unbox_value<hstring>(toggleBtn.Tag());
            bool state = toggleBtn.IsChecked().Value();
            StatusText().Text(L"Forms Command: " + tag + (state ? L" [Active]" : L" [Inactive]"));
            return;
        }

        StatusText().Text(L"Forms Command Executed: " + tag);
    }

    void MainWindow::OnBordersToolbarButtonClick(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = L"";

        if (auto btn = sender.try_as<Button>())
        {
            tag = unbox_value<hstring>(btn.Tag());
        }
        else if (auto item = sender.try_as<MenuFlyoutItem>())
        {
            tag = unbox_value<hstring>(item.Tag());
        }

        StatusText().Text(L"Borders Command Executed: " + tag);
    }

    void MainWindow::OnBordersComboBoxChanged(IInspectable const& sender, SelectionChangedEventArgs const&)
    {
        if (auto combo = sender.try_as<ComboBox>())
        {
            hstring category = unbox_value<hstring>(combo.Tag());
            if (auto selectedItem = combo.SelectedItem().try_as<ComboBoxItem>())
            {
                hstring val = unbox_value<hstring>(selectedItem.Content());
                StatusText().Text(L"Borders " + category + L" set to: " + val);
            }
        }
    }
}
