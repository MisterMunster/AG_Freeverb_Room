#include <JuceHeader.h>

int main (int argc, char* argv[])
{
    juce::ignoreUnused(argc, argv);
    juce::UnitTestRunner runner;
    runner.runAllTests();

    return 0;
}
