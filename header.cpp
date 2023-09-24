struct DNSHeader {
  private:
    int id;
    int flags;
    int num_questions;
    int num_answers;
    int num_authorities;
    int num_additionals;

  public:
    DNSHeader(int input_id, int input_flags) {
      id = input_id;
      flags = input_flags;
      num_questions = 0;
      num_answers = 0;
      num_authorities = 0;
      num_additionals = 0;
    }
};

int main() {}