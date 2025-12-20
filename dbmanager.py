import sqlite3

def SearchSticker(key):
    conn = sqlite3.connect("mydb.sqlite")
    cursor = conn.cursor()
    cursor.execute("""
        SELECT * FROM stickerdata;
    """)
    #conn.commit()
    rows = cursor.fetchall()
    result = []
    for row in rows:
        #print(row)
        if(row[1] == key):
            result.append(row)
    conn.close()
    if result == []:
        return "not found"
    else: return result

