import sqlite3

symbol = ['&',' ']

def SearchSticker(key):
    socre = 0   #部分匹配用的分數


    conn = sqlite3.connect("mydb.sqlite")
    cursor = conn.cursor()
    cursor.execute("""
        SELECT * FROM stickerdata;
    """)
    #conn.commit()
    rows = cursor.fetchall()
    result = []
    for row in rows:
        #匹配規則 start
        #print(row)
        #完全符合匹配
        #if(row[1] == key):
        #    result.append(row)

        mainnsubstring = {"main":None,"sub":None}
        #print(row)
 
        try:
            tempstr = row[1]    #tempstr = "嗯嗯,是呀"
            tempstr = tempstr.split(',')    #tempstr = ["嗯嗯","是呀"]
            #print(tempstr)

            #全部命中
            for item in tempstr:    #item = "嗯嗯"
                if(item == key):
                    result.append(row)
            #少部分不同
            #for item in tempstr:    #item = "嗯嗯"
            #    for ch in key:     #ch = '嗯'
            #        if ch in item:
            #            score += 1


        except Exception:
            pass


        #匹配規則 end
    conn.close()
    return result



if __name__ == "__main__":
    print("(debug)[dbmanager.main]start")
    print("search result:")
    userinput = ""
    while(userinput != "quit"):
        print("userinput: "+ userinput + "=",sep = "")
        result = SearchSticker(userinput)
        for it in result:
            print(it)
        userinput = input()
        
