from flask import Flask, request, abort

from linebot.v3 import (
    WebhookHandler
)
from linebot.v3.exceptions import (
    InvalidSignatureError
)
from linebot.v3.messaging import (
    Configuration,
    ApiClient,
    MessagingApi,
    ReplyMessageRequest,
    TextMessage,
    ImageMessage
)
from linebot.v3.webhooks import (
    MessageEvent,
    TextMessageContent
)

from linebot.models import(ImageSendMessage)

import dbmanager

app = Flask(__name__)

configuration = Configuration(access_token='CIKzKE9e67IxcBxJgqBDtFdtyf3qmcW9+EC6hwR1WKFc2V1KHlQPfWFjDzcMCTSaDHlchm/SZ1SKTOJhY1kWVqMD4OnuAbCa+ihyeYddAC0kct9kdY+SXHGBP9dwzxGAXCRKmWwEPyO0V0+0ySoa8QdB04t89/1O/w1cDnyilFU=')
handler = WebhookHandler('ff9fa0742ea2fbcee0a0a2bee3e150f2')


@app.route("/callback", methods=['POST'])
def callback():
    # get X-Line-Signature header value
    signature = request.headers['X-Line-Signature']

    # get request body as text
    body = request.get_data(as_text=True)
    app.logger.info("Request body: " + body)

    # handle webhook body
    try:
        handler.handle(body, signature)
    except InvalidSignatureError:
        app.logger.info("Invalid signature. Please check your channel access token/channel secret.")
        abort(400)

    return 'OK'


@handler.add(MessageEvent, message=TextMessageContent)
def handle_message(event):
    try:
        with ApiClient(configuration) as api_client:
            result = dbmanager.SearchSticker(event.message.text)
            if(result == "not found"):
                line_bot_api = MessagingApi(api_client)
                line_bot_api.reply_message_with_http_info(
                ReplyMessageRequest(
                    reply_token=event.reply_token,
                    #messages=[TextMessage(text=event.message.text)]    #respond一樣的
                    #messages=[TextMessage(text= dbmanager.SearchSticker(event.message.text))]  #response文字結果
                    messages=[TextMessage(text="not found")]
                )
            )
            else:
                line_bot_api = MessagingApi(api_client)
                line_bot_api.reply_message_with_http_info(
                    ReplyMessageRequest(
                        reply_token=event.reply_token,
                        #messages=[TextMessage(text=event.message.text)]    #respond一樣的
                        #messages=[TextMessage(text= dbmanager.SearchSticker(event.message.text))]  #response文字結果
                        messages=[ImageMessage(
                            original_content_url = result[2],
                            preview_image_url = result[2]
                            ),
                            TextMessage(text="來源:"+result[3])
                        ]
                    )
                )
        #original_content_url = "https://ithelp.ithome.com.tw/upload/images/20220925/20151681EaMkK6ROvq.jpg",
        #preview_image_url = "https://ithelp.ithome.com.tw/upload/images/20220925/20151681EaMkK6ROvq.jpg"
    except Exception as e:
        print("(error)[handle_message]e:")
        print(e)
        with ApiClient(configuration) as api_client:
            line_bot_api = MessagingApi(api_client)
            line_bot_api.reply_message_with_http_info(
                ReplyMessageRequest(
                    reply_token=event.reply_token,
                    #messages=[TextMessage(text=event.message.text)]    #respond一樣的
                    #messages=[TextMessage(text= dbmanager.SearchSticker(event.message.text))]  #response文字結果
                    messages=[TextMessage(text="哎呀!發生了一點小失誤")]
                )
            )

if __name__ == "__main__":
    app.run()