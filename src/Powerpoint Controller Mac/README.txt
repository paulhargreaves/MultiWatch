This is MAC only implementation of what came with the SDK.

Should be trivial to extend the existing windows one to understand the change
in protocol; specifically the watch sends a "random" number that is echo'd
back; if it is then the watch knows that everything is ok and sends the proper
request. If not the the watch buzzes to let the presenter know that something
is wrong.

Just use the ppt.py exactly as you would the original windows one that came
with the SDK.
