"""
  bridge.tx("regbot mclear\n");
  event.clearEvents();
  bridge.tx("regbot madd vel=0.2:time=1\n");
  bridge.tx("regbot madd tr=0.1:time=1,turn=-90\n");
  bridge.tx("regbot madd :time=1\n");
  bridge.tx("regbot start\n");
  event.waitForEvent(0);
"""

# read the GUI code
with open('mission_gui.txt') as f:
    lines = f.readlines()
lines = [line.rstrip('\n') for line in lines]

# convert the GUI code into cpp
code = []
code.append(r'bridge.tx("regbot mclear\n");')
code.append('event.clearEvents();')
for line in lines:
    codeline = 'bridge.tx("regbot madd ' + line + r'\n");'
    code.append(codeline)
code.append('event.waitForEvent(0);')

# write the python code to mission_py.txt file
with open("mission_cpp.txt", "w") as f:
    i = 0
    while i < len(code):
        codeline = code[i] + '\n'
        f.write(codeline)
        i += 1

print("Finished generating mission_py.txt")
