# ERROR message="Code Block not allowed here"
{}

# ERROR message="If Statement not allowed here"
if (true) {}

# ERROR message="For Loop not allowed here"
for (uint32 i = 0; i < 10; i++) {}

# ERROR message="While Loop not allowed here"
while (true) {}

# ERROR message="Do While Loop not allowed here"
do {} while (true)

# ERROR message="Expression not allowed here"
+32

# This should be allowed
const uint32 GLOBAL_VAR = 0

# This should be allowed
struct StructType {}

# This should be allowed
void main() {

}