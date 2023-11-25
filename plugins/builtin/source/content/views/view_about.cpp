#include "content/views/view_about.hpp"

#include <hex/api_urls.hpp>
#include <hex/api/content_registry.hpp>
#include <hex/api/achievement_manager.hpp>

#include <hex/helpers/fmt.hpp>
#include <hex/helpers/fs.hpp>
#include <hex/helpers/utils.hpp>
#include <hex/helpers/http_requests.hpp>

#include <content/popups/popup_docs_question.hpp>

#include <romfs/romfs.hpp>
#include <wolv/utils/string.hpp>

namespace hex::plugin::builtin {

    ViewAbout::ViewAbout() : View::Modal("hex.builtin.view.help.about.name") {
        // Add "About" menu item to the help menu
        ContentRegistry::Interface::addMenuItem({ "hex.builtin.menu.help", "hex.builtin.view.help.about.name" }, 1000, Shortcut::None, [this] {
            this->getWindowOpenState() = true;
        });

        ContentRegistry::Interface::addMenuItemSeparator({ "hex.builtin.menu.help" }, 2000);

        // Add documentation links to the help menu
        ContentRegistry::Interface::addMenuItem({ "hex.builtin.menu.help", "hex.builtin.view.help.documentation" }, 3000, Shortcut::None, [] {
            hex::openWebpage("https://docs.werwolv.net/imhex");
            AchievementManager::unlockAchievement("hex.builtin.achievement.starting_out", "hex.builtin.achievement.starting_out.docs.name");
        });

        ContentRegistry::Interface::addMenuItem({ "hex.builtin.menu.help", "hex.builtin.menu.help.ask_for_help" }, 4000, CTRLCMD + SHIFT + Keys::D, [] {
            PopupDocsQuestion::open();
        });
    }


    void ViewAbout::drawAboutMainPage()
    {
        // Draw main about table
        if (ImGui::BeginTable("about_table", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // Draw the ImHex icon
            if (!this->m_logoTexture.isValid())
                this->m_logoTexture = ImGuiExt::Texture(romfs::get("assets/common/logo.png").span());

            ImGui::Image(this->m_logoTexture, scaled({ 100, 100 }));
            if (ImGui::IsItemHovered() && ImGui::IsItemClicked()) {
                this->m_clickCount += 1;
            }
            ImGui::TableNextColumn();

            ImGuiExt::BeginSubWindow("Build Information", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
            {
                if (ImGui::BeginTable("Information", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    {
                        // Draw basic information about ImHex and its version
                        ImGuiExt::TextFormatted("ImHex Hex Editor v{} by WerWolv  " ICON_FA_CODE_BRANCH, ImHexApi::System::getImHexVersion());

                        ImGui::SameLine();

                        // Draw a clickable link to the current commit
                        if (ImGuiExt::Hyperlink(hex::format("{0}@{1}", ImHexApi::System::getCommitBranch(), ImHexApi::System::getCommitHash()).c_str()))
                            hex::openWebpage("https://github.com/WerWolv/ImHex/commit/" + ImHexApi::System::getCommitHash(true));
                    }

                    ImGui::TableNextColumn();
                    {
                        // Draw the build date and time
                        ImGuiExt::TextFormatted("Compiled on {} at {}", __DATE__, __TIME__);
                    }

                    ImGui::TableNextColumn();
                    {
                        // Draw the author of the current translation
                        ImGui::TextUnformatted("hex.builtin.view.help.about.translator"_lang);
                    }

                    ImGui::TableNextColumn();
                    {
                        // Draw information about the open-source nature of ImHex
                        ImGui::TextUnformatted("hex.builtin.view.help.about.source"_lang);

                        ImGui::SameLine();

                        // Draw a clickable link to the GitHub repository
                        if (ImGuiExt::Hyperlink("WerWolv/ImHex"))
                            hex::openWebpage("https://github.com/WerWolv/ImHex");
                    }
                    ImGui::EndTable();
                }
            }
            ImGuiExt::EndSubWindow();

            ImGui::EndTable();
        }

        // Draw donation links
        ImGuiExt::Header("hex.builtin.view.help.about.donations"_lang);

        ImGuiExt::TextFormattedWrapped("{}", static_cast<const char *>("hex.builtin.view.help.about.thanks"_lang));

        ImGui::NewLine();

        struct DonationPage {
            ImGuiExt::Texture texture;
            const char *link;
        };

        static std::array DonationPages = {
            DonationPage { ImGuiExt::Texture(romfs::get("assets/common/donation/paypal.png").span<std::byte>()),  "https://werwolv.net/donate"           },
            DonationPage { ImGuiExt::Texture(romfs::get("assets/common/donation/github.png").span<std::byte>()),  "https://github.com/sponsors/WerWolv"  },
            DonationPage { ImGuiExt::Texture(romfs::get("assets/common/donation/patreon.png").span<std::byte>()), "https://patreon.com/werwolv"          },
        };

        ImGui::NewLine();
        if (ImGui::BeginTable("DonationLinks", 5, ImGuiTableFlags_SizingStretchSame, ImGui::GetContentRegionAvail())) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            for (const auto &page : DonationPages) {
                ImGui::TableNextColumn();

                const auto size = page.texture.getSize() / 1.5F;
                const auto startPos = ImGui::GetCursorScreenPos();
                ImGui::Image(page.texture, page.texture.getSize() / 1.5F);

                if (ImGui::IsItemHovered()) {
                    ImGui::GetForegroundDrawList()->AddShadowCircle(startPos + size / 2, size.x / 2, ImGui::GetColorU32(ImGuiCol_Button), 100.0F, ImVec2(), ImDrawFlags_ShadowCutOutShapeBackground);
                }

                if (ImGui::IsItemClicked()) {
                    hex::openWebpage(page.link);
                }
            }

            ImGui::EndTable();
        }
    }

    void ViewAbout::drawContributorPage() {
        struct Contributor {
            const char *name;
            const char *description;
            const char *link;
            bool mainContributor;
        };

        constexpr static std::array Contributors = {
            Contributor { "iTrooz", "A huge amount of help maintaining ImHex and the CI", "https://github.com/iTrooz", true },
            Contributor { "jumanji144", "A ton of help with the Pattern Language, API and usage stats", "https://github.com/Nowilltolife", true },
            Contributor { "Mary", "Porting ImHex to macOS originally", "https://github.com/marysaka", false },
            Contributor { "Roblabla", "Adding the MSI Windows installer", "https://github.com/roblabla", false },
            Contributor { "jam1garner", "Adding support for Rust plugins", "https://github.com/jam1garner", false },
            Contributor { "All other amazing contributors", "Being part of the community, opening issues, PRs and donating", "https://github.com/WerWolv/ImHex/graphs/contributors", false }
        };

        ImGuiExt::TextFormattedWrapped("These amazing people have contributed some incredible things to ImHex in the past.\nConsider opening a PR on the Git Repository to become a part of them!");
        ImGui::NewLine();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
        ImGuiExt::BeginSubWindow("Contributors", ImVec2(ImGui::GetContentRegionAvail().x, 0), ImGuiChildFlags_AutoResizeX);
        ImGui::PopStyleVar();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, scaled({ 5, 5 }));
        {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2());
            const auto width = ImGui::GetContentRegionAvail().x;
            if (ImGui::BeginTable("Contributors", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                for (const auto &contributor : Contributors) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(contributor.mainContributor ? ImGuiCol_PlotHistogram : ImGuiCol_ChildBg) & 0x1FFFFFFF);

                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2_scaled);
                    if (ImGui::BeginChild(contributor.name, ImVec2(width - 2, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) {
                        if (ImGuiExt::Hyperlink(contributor.name))
                            hex::openWebpage(contributor.link);

                        ImGui::Indent();
                        ImGui::TextUnformatted(contributor.description);
                        ImGui::Unindent();
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar();

                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
            ImGui::PopStyleVar();
        }
        ImGuiExt::EndSubWindow();
        ImGui::PopStyleVar();
    }

    void ViewAbout::drawLibraryCreditsPage() {

        struct Library {
            const char *name;
            const char *author;
            const char *link;
        };

        constexpr static std::array ImGuiLibraries = {
            Library { "ImGui", "ocornut", "https://github.com/ocornut/imgui" },
            Library { "ImPlot", "epezent", "https://github.com/epezent/implot" },
            Library { "imnodes", "Nelarius", "https://github.com/Nelarius/imnodes" },
            Library { "ImGuiColorTextEdit", "BalazsJako", "https://github.com/BalazsJako/ImGuiColorTextEdit" },

        };

        constexpr static std::array ExternalLibraries = {
            Library { "PatternLanguage", "WerWolv", "https://github.com/WerWolv/PatternLanguage" },
            Library { "libwolv", "WerWolv", "https://github.com/WerWolv/libwolv" },
            Library { "libromfs", "WerWolv", "https://github.com/WerWolv/libromfs" },
        };

        constexpr static std::array ThirdPartyLibraries = {
            Library { "capstone", "aquynh", "https://github.com/aquynh/capstone" },
            Library { "json", "nlohmann", "https://github.com/nlohmann/json" },
            Library { "yara", "VirusTotal", "https://github.com/VirusTotal/yara" },
            Library { "nativefiledialog-extended", "btzy", "https://github.com/btzy/nativefiledialog-extended" },
            Library { "microtar", "rxi", "https://github.com/rxi/microtar" },
            Library { "xdgpp", "danyspin97", "https://sr.ht/~danyspin97/xdgpp" },
            Library { "freetype", "freetype", "https://gitlab.freedesktop.org/freetype/freetype" },
            Library { "mbedTLS", "ARMmbed", "https://github.com/ARMmbed/mbedtls" },
            Library { "curl", "curl", "https://github.com/curl/curl" },
            Library { "fmt", "fmtlib", "https://github.com/fmtlib/fmt" },
            Library { "file", "file", "https://github.com/file/file" },
            Library { "glfw", "glfw", "https://github.com/glfw/glfw" },
            Library { "llvm", "llvm-project", "https://github.com/llvm/llvm-project" },
        };

        constexpr static auto drawTable = [](const char *category, const auto &libraries) {
            const auto width = ImGui::GetContentRegionAvail().x;
            ImGuiExt::BeginSubWindow(category);
            {
                for (const auto &library : libraries) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 50);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, scaled({ 12, 3 }));

                    if (ImGui::BeginChild(library.link, ImVec2(), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) {
                        if (ImGuiExt::Hyperlink(hex::format("{}/{}", library.author, library.name).c_str())) {
                            hex::openWebpage(library.link);
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();
                    if (ImGui::GetCursorPosX() > (width - 100_scaled))
                        ImGui::NewLine();

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(2);
                }
            }
            ImGuiExt::EndSubWindow();

            ImGui::NewLine();
        };

        ImGuiExt::TextFormattedWrapped("ImHex builds on top of the amazing work of a ton of talented library developers without which this project wouldn't stand.");
        ImGui::NewLine();

        drawTable("ImGui", ImGuiLibraries);
        drawTable("External", ExternalLibraries);
        drawTable("Third Party", ThirdPartyLibraries);
    }

    void ViewAbout::drawPathsPage() {
        constexpr static std::array<std::pair<const char *, fs::ImHexPath>, size_t(fs::ImHexPath::END) - 1U> PathTypes = {
            {
                { "Patterns", fs::ImHexPath::Patterns },
                { "Patterns Includes", fs::ImHexPath::PatternsInclude },
                { "Magic", fs::ImHexPath::Magic },
                { "Plugins", fs::ImHexPath::Plugins },
                { "Libraries", fs::ImHexPath::Libraries },
                { "Yara Patterns", fs::ImHexPath::Yara },
                { "Config", fs::ImHexPath::Config },
                { "Resources", fs::ImHexPath::Resources },
                { "Constants lists", fs::ImHexPath::Constants },
                { "Custom encodings", fs::ImHexPath::Encodings },
                { "Logs", fs::ImHexPath::Logs },
                { "Recent files", fs::ImHexPath::Recent },
                { "Scripts", fs::ImHexPath::Scripts },
                { "Themes", fs::ImHexPath::Themes },
                { "Data inspector scripts", fs::ImHexPath::Inspectors },
                { "Custom data processor nodes", fs::ImHexPath::Nodes },
            }
        };

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
        ImGuiExt::BeginSubWindow("Paths", ImGui::GetContentRegionAvail());
        {
            if (ImGui::BeginTable("##imhex_paths", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Paths");

                // Draw the table
                ImGui::TableHeadersRow();
                for (const auto &[name, type] : PathTypes) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(name);

                    ImGui::TableNextColumn();
                    for (auto &path : fs::getDefaultPaths(type, true)){
                        // Draw hyperlink to paths that exist or red text if they don't
                        if (wolv::io::fs::isDirectory(path)){
                            if (ImGuiExt::Hyperlink(wolv::util::toUTF8String(path).c_str())) {
                                fs::openFolderExternal(path);
                            }
                        } else {
                            ImGuiExt::TextFormattedColored(ImGuiExt::GetCustomColorVec4(ImGuiCustomCol_ToolbarRed), wolv::util::toUTF8String(path));
                        }
                    }
                }

                ImGui::EndTable();
            }
        }
        ImGuiExt::EndSubWindow();
        ImGui::PopStyleVar();

    }

    void ViewAbout::drawReleaseNotesPage() {
        static std::string releaseTitle;
        static std::vector<std::string> releaseNotes;

        // Set up the request to get the release notes the first time the page is opened
        AT_FIRST_TIME {
            static HttpRequest request("GET", GitHubApiURL + std::string("/releases/tags/v") + ImHexApi::System::getImHexVersion(false));

            this->m_releaseNoteRequest = request.execute();
        };

        // Wait for the request to finish and parse the response
        if (this->m_releaseNoteRequest.valid()) {
            if (this->m_releaseNoteRequest.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto response = this->m_releaseNoteRequest.get();
                nlohmann::json json;

                if (response.isSuccess()) {
                    // A valid response was received, parse it
                    try {
                        json = nlohmann::json::parse(response.getData());

                        // Get the release title
                        releaseTitle = json["name"].get<std::string>();

                        // Get the release notes and split it into lines
                        auto body = json["body"].get<std::string>();
                        releaseNotes = wolv::util::splitString(body, "\r\n");
                    } catch (std::exception &e) {
                        releaseNotes.push_back("## Error: " + std::string(e.what()));
                    }
                } else {
                    // An error occurred, display it
                    releaseNotes.push_back("## HTTP Error: " + std::to_string(response.getStatusCode()));
                }
            } else {
                // Draw a spinner while the release notes are loading
                ImGuiExt::TextSpinner("hex.builtin.common.loading"_lang);
            }
        }


        // Function to handle drawing of a regular text line
        static const auto drawRegularLine = [](const std::string &line) {
            ImGui::Bullet();
            ImGui::SameLine();

            // Check if the line contains bold text
            auto boldStart = line.find("**");
            if (boldStart != std::string::npos) {
                // Find the end of the bold text
                auto boldEnd = line.find("**", boldStart + 2);

                // Draw the line with the bold text highlighted
                ImGui::TextUnformatted(line.substr(0, boldStart).c_str());
                ImGui::SameLine(0, 0);
                ImGuiExt::TextFormattedColored(ImGuiExt::GetCustomColorVec4(ImGuiCustomCol_Highlight), "{}", line.substr(boldStart + 2, boldEnd - boldStart - 2).c_str());
                ImGui::SameLine(0, 0);
                ImGui::TextUnformatted(line.substr(boldEnd + 2).c_str());
            } else {
                // Draw the line normally
                ImGui::TextUnformatted(line.c_str());
            }
        };

        // Draw the release title
        if (!releaseTitle.empty()) {
            auto title = hex::format("v{}: {}", ImHexApi::System::getImHexVersion(false), releaseTitle);
            ImGuiExt::Header(title.c_str(), true);
            ImGui::Separator();
        }

        // Draw the release notes and format them using parts of the GitHub Markdown syntax
        // This is not a full implementation of the syntax, but it's enough to make the release notes look good.
        for (const auto &line : releaseNotes) {
            if (line.starts_with("## ")) {
                // Draw H2 Header
                ImGuiExt::Header(line.substr(3).c_str());
            } else if (line.starts_with("### ")) {
                // Draw H3 Header
                ImGuiExt::Header(line.substr(4).c_str());
            } else if (line.starts_with("- ")) {
                // Draw bullet point
                drawRegularLine(line.substr(2));
            } else if (line.starts_with("    - ")) {
                // Draw further indented bullet point
                ImGui::Indent();
                ImGui::Indent();
                drawRegularLine(line.substr(6));
                ImGui::Unindent();
                ImGui::Unindent();
            }
        }
    }

    void ViewAbout::drawCommitHistoryPage() {
        struct Commit {
            std::string hash;
            std::string message;
            std::string description;
            std::string author;
            std::string date;
            std::string url;
        };

        static std::vector<Commit> commits;

        // Set up the request to get the commit history the first time the page is opened
        AT_FIRST_TIME {
            static HttpRequest request("GET", GitHubApiURL + std::string("/commits?per_page=100"));
            this->m_commitHistoryRequest = request.execute();
        };

        // Wait for the request to finish and parse the response
        if (this->m_commitHistoryRequest.valid()) {
            if (this->m_commitHistoryRequest.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto response = this->m_commitHistoryRequest.get();
                nlohmann::json json;

                if (response.isSuccess()) {
                    // A valid response was received, parse it
                    try {
                        json = nlohmann::json::parse(response.getData());

                        for (auto &commit : json) {
                            const auto message = commit["commit"]["message"].get<std::string>();

                            // Split commit title and description. They're separated by two newlines.
                            const auto messageEnd = message.find("\n\n");

                            auto commitTitle        = messageEnd == std::string::npos ? message : message.substr(0, messageEnd);
                            auto commitDescription  = messageEnd == std::string::npos ? "" : message.substr(commitTitle.size() + 2);

                            auto url    = commit["html_url"].get<std::string>();
                            auto sha    = commit["sha"].get<std::string>();
                            auto date   = commit["commit"]["author"]["date"].get<std::string>();
                            auto author = hex::format("{} <{}>",
                                                      commit["commit"]["author"]["name"].get<std::string>(),
                                                      commit["commit"]["author"]["email"].get<std::string>()
                                          );

                            // Move the commit data into the list of commits
                            commits.emplace_back(
                                std::move(sha),
                                std::move(commitTitle),
                                std::move(commitDescription),
                                std::move(author),
                                std::move(date),
                                std::move(url)
                            );
                        }

                    } catch (std::exception &e) {
                        commits.emplace_back(
                            "hex.builtin.common.error"_lang,
                            e.what(),
                            "",
                            "",
                            ""
                        );
                    }
                } else {
                    // An error occurred, display it
                    commits.emplace_back(
                        "hex.builtin.common.error"_lang,
                        "HTTP " + std::to_string(response.getStatusCode()),
                        "",
                        "",
                        ""
                    );
                }
            } else {
                // Draw a spinner while the commits are loading
                ImGuiExt::TextSpinner("hex.builtin.common.loading"_lang);
            }
        }

        // Draw commits table
        if (!commits.empty()) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
            ImGuiExt::BeginSubWindow("Commits", ImGui::GetContentRegionAvail());
            {

                if (ImGui::BeginTable("##commits", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                    // Draw commits
                    for (const auto &commit : commits) {
                        ImGui::PushID(commit.hash.c_str());
                        ImGui::TableNextRow();

                        // Draw hover tooltip
                        ImGui::TableNextColumn();
                        if (ImGui::Selectable("##commit", false, ImGuiSelectableFlags_SpanAllColumns)) {
                            hex::openWebpage(commit.url);
                        }

                        if (ImGui::IsItemHovered()) {
                            if (ImGui::BeginTooltip()) {
                                // Draw author and commit date
                                ImGuiExt::TextFormattedColored(ImGuiExt::GetCustomColorVec4(ImGuiCustomCol_Highlight), "{}", commit.author);
                                ImGui::SameLine();
                                ImGuiExt::TextFormatted("@ {}", commit.date.c_str());

                                // Draw description if there is one
                                if (!commit.description.empty()) {
                                    ImGui::Separator();
                                    ImGuiExt::TextFormatted("{}", commit.description);
                                }

                                ImGui::EndTooltip();
                            }

                        }

                        // Draw commit hash
                        ImGui::SameLine(0, 0);
                        ImGuiExt::TextFormattedColored(ImGuiExt::GetCustomColorVec4(ImGuiCustomCol_Highlight), "{}", commit.hash.substr(0, 7));

                        // Draw the commit message
                        ImGui::TableNextColumn();

                        const ImColor color = [&]{
                            if (commit.hash == ImHexApi::System::getCommitHash(true))
                                return ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
                            else
                                return ImGui::GetStyleColorVec4(ImGuiCol_Text);
                        }();
                        ImGuiExt::TextFormattedColored(color, commit.message);

                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }
            }
            ImGuiExt::EndSubWindow();
            ImGui::PopStyleVar();
        }
    }

    void ViewAbout::drawLicensePage() {
        ImGuiExt::TextFormattedWrapped("{}", romfs::get("licenses/LICENSE").string());
    }

    void ViewAbout::drawAboutPopup() {
        struct Tab {
            using Function = void (ViewAbout::*)();

            const char *unlocalizedName;
            Function function;
        };

        constexpr std::array Tabs = {
            Tab { "ImHex",                                      &ViewAbout::drawAboutMainPage       },
            Tab { "hex.builtin.view.help.about.contributor",    &ViewAbout::drawContributorPage     },
            Tab { "hex.builtin.view.help.about.libs",           &ViewAbout::drawLibraryCreditsPage  },
            Tab { "hex.builtin.view.help.about.paths",          &ViewAbout::drawPathsPage           },
            Tab { "hex.builtin.view.help.about.release_notes",  &ViewAbout::drawReleaseNotesPage    },
            Tab { "hex.builtin.view.help.about.commits",        &ViewAbout::drawCommitHistoryPage   },
            Tab { "hex.builtin.view.help.about.license",        &ViewAbout::drawLicensePage         },
        };

        // Allow the window to be closed by pressing ESC
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
            ImGui::CloseCurrentPopup();

        if (ImGui::BeginTabBar("about_tab_bar")) {
            // Draw all tabs
            for (const auto &[unlocalizedName, function] : Tabs) {
                if (ImGui::BeginTabItem(Lang(unlocalizedName))) {
                    ImGui::NewLine();

                    if (ImGui::BeginChild(1)) {
                        (this->*function)();
                    }
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
    }

    void ViewAbout::drawContent() {
        this->drawAboutPopup();
    }

}
