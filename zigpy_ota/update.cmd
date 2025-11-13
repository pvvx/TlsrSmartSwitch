echo [] > index.json
for %%a in (../bin/1141-02??-0???3001-*.zigbee) do (
start /wait node scripts/add.js ../bin/%%a
)
copy /Y index.json ..\bin\index%1.json

del *.zigbee
del index.json



