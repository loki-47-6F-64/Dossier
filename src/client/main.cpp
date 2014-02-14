namespace dossier { namespace client {
extern int start(int argc, char *argv[]);
};};

int main(int argc, char *argv[]) {
  return dossier::client::start(argc, argv);
}
