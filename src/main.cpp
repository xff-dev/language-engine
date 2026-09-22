#include "CLI/CLI.hpp"
#include "app/app.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[]) {
  CLI cli(argc, argv);
  CliOptions options;
  
  try {
    options = cli.parse();
  } catch (std::runtime_error error) {
    std::cout << error.what() << std::endl;
    options.help = true;
  }
  
  options.programName = argv[0];

  App app(options);
  return app.run();
}
