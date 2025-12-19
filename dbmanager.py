import sqlite3

def SearchSticker(key):
    conn = sqlite3.connect("mydb.sqlite")
    cursor = conn.cursor()
    cursor.execute("""
        SELECT * FROM stickerdata;
    """)
    #conn.commit()
    rows = cursor.fetchall()
    for row in rows:
        #print(row)
        if(row[1] == key):
            conn.close()
            return row
    conn.close()
    return "not found"

