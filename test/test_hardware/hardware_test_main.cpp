// Only define implementation and signals, other configs in platformio.ini
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <Arduino.h>


// // Simple custom reporter that outputs to Serial
// struct SerialReporter : public doctest::IReporter {
//     // Constructor - initialize any variables here
//     SerialReporter(const doctest::ContextOptions& options) {}
    
//     // Destructor
//     virtual ~SerialReporter() {}
    
//     // Called when a test case is about to start
//     void test_case_start(const doctest::TestCaseData& test_case_data) override {
//         Serial.print("\n");
//         Serial.print("┌─────────────────────────────────────────────────────────────┐\n");
//         Serial.printf("│ RUNNING TEST: %-50s │\n", test_case_data.m_name);
//         Serial.print("└─────────────────────────────────────────────────────────────┘\n");
        
//         // Store start time for duration calculation
//         _test_case_start_time = micros();
//     }
    
//     // Called when a test case has ended
//     void test_case_end(const doctest::CurrentTestCaseStats& stats) override {
//         unsigned long duration = micros() - _test_case_start_time;
        
//         Serial.print("┌─────────────────────────────────────────────────────────────┐\n");
//         Serial.printf("│ RESULT: %-10s │ TIME: %-8lu µs │ ASSERTIONS: %-3d │\n", 
//                     stats.failure_flags ? "FAILED" : "PASSED", 
//                     duration, 
//                     stats.numAssertsCurrentTest);
//         Serial.print("└─────────────────────────────────────────────────────────────┘\n");
//     }
    
//     // Called when a subcase is about to start
//     void subcase_start(const doctest::SubcaseSignature& signature) override {
//         Serial.printf("  ▶ %s\n", signature.m_name);
//     }
    
//     // Called when a subcase has ended
//     void subcase_end() override {}
    
//     // Called when an assertion has failed
//     void log_assert(const doctest::AssertData& assert_data) override {
//         if(!assert_data.m_failed)
//             return;
            
//         Serial.printf("    ❌ FAILED: %s\n", assert_data.m_expr);
//         Serial.printf("      at line %d in %s\n", 
//                     assert_data.m_line, 
//                     assert_data.m_file);
//     }
    
//     // Called when a message is logged from the test
//     void log_message(const doctest::MessageData& message_data) override {
//         Serial.printf("    %s\n", message_data.m_string);
//     }
    
//     // Called when all tests have completed
//     void test_run_end(const doctest::TestRunStats& stats) override {
//         Serial.print("\n");
//         Serial.print("┌─────────────────────────────────────────────────────────────┐\n");
//         Serial.printf("│ SUMMARY: %3d tests │ %3d passed │ %3d failed │              │\n",
//                     stats.numTestCases,
//                     stats.numTestCases - stats.numTestCasesFailed,
//                     stats.numTestCasesFailed);
//         Serial.printf("│          %3d asserts │ %3d passed │ %3d failed │              │\n",
//                     stats.numAsserts,
//                     stats.numAsserts - stats.numAssertsFailed,
//                     stats.numAssertsFailed);
//         Serial.print("└─────────────────────────────────────────────────────────────┘\n");
//     }
    
//     // Additional required overrides
//     void test_case_reenter(const doctest::TestCaseData&) override {}
//     void test_case_exception(const doctest::TestCaseException&) override {}
//     void test_case_skipped(const doctest::TestCaseData&) override {}
    
//     // Other required overrides
//     void report_query(const doctest::QueryData&) override {}
//     void test_run_start() override {}
    
// private:
//     unsigned long _test_case_start_time = 0;
// };

// // Register the reporter with doctest - adding priority parameter (1 = high priority)
// DOCTEST_REGISTER_REPORTER("serial", 1, SerialReporter);

int main(int argc, char** argv) {
    // Hardware setup
    Serial.begin(115200);
    delay(1000);  // Give serial time to connect

    if (CrashReport) Serial.print(CrashReport);    
    
    Serial.println("\n=== Starting Hardware Tests ===");

    // Configure test runner
    doctest::Context context;
    
    // Set the reporter to use
   // context.setOption("r", "serial");
    
    // Minimize output noise
    context.setOption("no-version", true);
    context.setOption("no-intro", true);
    context.setOption("no-subcase-events", true);
    context.setOption("no-path-filenames", true);
    context.setOption("no-line-numbers", true);
    context.setOption("no-skip", true);
    context.setOption("duration", false);
    
    // Run tests
    int res = context.run();
    Serial.printf("\nTests complete with result: %d\n", res);
    
    // Keep USB alive
    while(1) { delay(100); }
    return res;
} 