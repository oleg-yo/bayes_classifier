#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#define message_classes_amount 3

using namespace std;

struct Message_class {
    string name;
    double probability;
};
struct Message {
    void addWord(string word) {
        word.erase(remove_if(word.begin(), word.end(), [] (char c) {
            return c == ',' || c == '.' || c == '?' || c == '!' || c == '"' || c == ':';
        }), word.end());
        transform(word.begin(), word.end(), word.begin(), ::tolower);

        words.push_back(word);
    }

    vector <string> words;
    unsigned char class_id;
};
struct Dictionary_word {
    vector <int> amount;
    vector <double> probability_norm;
};

int main()
{
    vector <Message_class> message_classes = {
        {"STORY", 1.0 / message_classes_amount},
        {"SPAM", 1.0 / message_classes_amount},
        {"CODE", 1.0 / message_classes_amount}
    };

    vector <Message> input_messages;
    for (int i = 0; i < message_classes_amount; i++) {
        ifstream this_class_input_file("dataset/" + message_classes[i].name + ".txt");

        Message this_message;
        this_message.class_id = i;
        int this_message_size_begin = this_message.words.size();
        for (string word; this_class_input_file >> word; ) {
            this_message.addWord(word);
        }
        input_messages.push_back(this_message);

        cout << this_message.words.size() - this_message_size_begin << " words from " << message_classes[i].name << " were analyzed" << endl;
    }

    unordered_map <string, Dictionary_word> dictionary;
    for (auto &input_message : input_messages) {
        for (auto &word : input_message.words) {
            Dictionary_word dictionary_word;
            dictionary_word.amount.resize(message_classes.size());
            dictionary_word.probability_norm.resize(message_classes.size());
            dictionary_word.amount[input_message.class_id] = 0;

            auto this_word_status = dictionary.insert({word, dictionary_word});
            auto this_word_in_dictionary = this_word_status.first;
            this_word_in_dictionary->second.amount[input_message.class_id] += 1;
        }
    }
    cout << endl << "Dictionary with " << dictionary.size() << " words has been created" << endl;

    for (auto &word : dictionary) {
        double word_total_amount = 0;
        for (auto &n : word.second.amount)
            word_total_amount += n;
        for (int i = 0; i < message_classes_amount; i++) {
            word.second.probability_norm[i] = (word.second.amount[i] + message_classes[i].probability) / (word_total_amount + 1.0);
        }
    }
    cout << "Classifier was taught" << endl;

    cout << endl << "Reading test message" << endl;
    ifstream input_test("test/story_test.txt");
    Message test_message;
    for (string word; input_test >> word; ) {
        test_message.addWord(word);
    }
    cout << "Test message has " << test_message.words.size() << " words" << endl;
    vector <double> test_message_classes_probability (message_classes_amount);
    for (int i = 0; i < message_classes_amount; i++) {
        test_message_classes_probability[i] = message_classes[i].probability;
    }
    for (auto &word : test_message.words) {
        auto this_word_in_dictionary = dictionary.find(word);
        vector <double> this_word_probability_norm (message_classes_amount);
        if (this_word_in_dictionary == dictionary.end())
            fill(this_word_probability_norm.begin(), this_word_probability_norm.end(), 1.0 / message_classes_amount);
        else
            this_word_probability_norm = this_word_in_dictionary->second.probability_norm;

        for (int i = 0; i < message_classes_amount; i++) {
            test_message_classes_probability[i] *= this_word_probability_norm[i];
        }
    }
    cout << "Message analyzing has finished" << endl;

    cout << endl << "Classes probabilities: " << fixed << setprecision(40) << endl;
    for (int i = 0; i < message_classes_amount; i++) {
        cout << "P(" << message_classes[i].name << ") " << setw(10 - message_classes[i].name.length()) << "= " << test_message_classes_probability[i] << endl;
    }
    int max_probability_index = max_element(test_message_classes_probability.begin(), test_message_classes_probability.end()) - test_message_classes_probability.begin();
    cout << endl << "Test message is " << message_classes[max_probability_index].name << endl;

    return 0;
}
