#include "ipfs-daemon.h"
#include "mainwindow.h"
#include "option-group.h"
#include "project_config.h"

#include <gtkmm/application.h>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

/**
 * \brief Entry point of the app
 */
int main(int argc, char* argv[])
{
  // Set the command-line parameters option settings
  Glib::OptionContext context("LibreWeb Browser - Decentralized Web Browser");
  OptionGroup group;
  Glib::ustring default_timeout = group.timeout;
  context.set_main_group(group);

  // Create the GTK application
  auto app = Gtk::Application::create();
  app->set_flags(Gio::ApplicationFlags::APPLICATION_NON_UNIQUE);

  // Parse the context
  try
  {
    context.parse(argc, argv);
    if (group.version)
    {
      std::cout << "LibreWeb Browser " << PROJECT_VER << std::endl;
      exit(EXIT_SUCCESS);
    }
  }
  catch (const Glib::Error& error)
  {
    std::cerr << "ERROR: Parse failure: " << error.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  // The default is to start the IPFS Daemon
  if (group.disable_ipfs_daemon)
  {
    std::cout << "WARN: You disabled the IPFS Daemon from starting-up "
                 "(you are using: -d/--disable-ipfs-daemon)."
              << std::endl;
  }
  else
  {
    IPFSDaemon ipfs_daemon;
    ipfs_daemon.spawn();
  }

  if (group.timeout.compare(default_timeout) != 0)
  {
    std::cout << "WARN: You changed the time-out value. Please, be sure you know what you're doing." << std::endl;
  }

  // Run the GTK main window in the main thread
  MainWindow main_window(group.timeout);
  return app->run(main_window);
}
