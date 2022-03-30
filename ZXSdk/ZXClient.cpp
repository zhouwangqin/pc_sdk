#include "pch.h"
#include "ZXClient.h"
#include "ZXEngine.h"
#include "easywsclient.hpp"
#include "rtc_base/strings/json.h"

#include <cstdlib>
#define random(a,b) (rand()%(b-a)+a)

static ZXClient* pClient = nullptr;

ZXClient::ZXClient()
{
	pClient = this;

	websocket_ = nullptr;
	pZXEngine = nullptr;

	nIndex = 0;
	nType = -1;
	nRespOK = -1;
	bRespResult = false;
	bClose = false;
}

ZXClient::~ZXClient()
{
	
}

void OnDataRecv(const std::string& message)
{
	RTC_LOG(LS_ERROR) << "websocket recv = " << message;

	if (pClient == nullptr)
	{
		return;
	}
	if (pClient->bClose)
	{
		return;
	}

	Json::Value jRoot;
	Json::Reader reader;
	if (!reader.parse(message, jRoot))

	//Json::Value jRoot;
	//Json::String jErrs;
	//Json::CharReaderBuilder jBuilder;
	//std::unique_ptr<Json::CharReader> jReader(jBuilder.newCharReader());
	//if (!jReader->parse(message.c_str(), message.c_str() + message.length(), &jRoot, &jErrs))
	{
		RTC_LOG(LS_ERROR) << "recv data = " << message << ", error = ";// << jErrs;
		return;
	}

	bool bResp = false;
	rtc::GetBoolFromJsonObject(jRoot, "response", &bResp);
	if (bResp)
	{
		int nId = 0;
		if (!rtc::GetIntFromJsonObject(jRoot, "id", &nId))
		{
			RTC_LOG(LS_ERROR) << "recv data id error";
			return;
		}

		if (nId == pClient->nIndex)
		{
			bool bOK = false;
			if (!rtc::GetBoolFromJsonObject(jRoot, "ok", &bOK))
			{
				RTC_LOG(LS_ERROR) << "recv data ok error";
				return;
			}

			if (bOK)
			{
				pClient->nRespOK = 1;
				if (pClient->nType == 10000)
				{
					pClient->bRespResult = true;

					Json::Value jData;
					if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
					{
						RTC_LOG(LS_ERROR) << "recv data data error";
						return;
					}

					Json::Value jUsers;
					if (!rtc::GetValueFromJsonObject(jData, "users", &jUsers))
					{
						RTC_LOG(LS_ERROR) << "recv data users error";
						return;
					}

					Json::Value jPubs;
					if (!rtc::GetValueFromJsonObject(jData, "pubs", &jPubs))
					{
						RTC_LOG(LS_ERROR) << "recv data pubs error";
						return;
					}

					std::vector<Json::Value> jUser;
					if (!rtc::JsonArrayToValueVector(jUsers, &jUser))
					{
						RTC_LOG(LS_ERROR) << "recv data users error";
						return;
					}

					for (size_t i = 0; i < jUser.size(); i++)
					{
						pClient->pZXEngine->respPeerJoin(jUser[i]);
					}

					std::vector<Json::Value> jPub;
					if (!rtc::JsonArrayToValueVector(jPubs, &jPub))
					{
						RTC_LOG(LS_ERROR) << "recv data pubs error";
						return;
					}

					for (size_t i = 0; i < jPub.size(); i++)
					{
						pClient->pZXEngine->respStreamAdd(jPub[i]);
					}
				}
				if (pClient->nType == 10002)
				{
					pClient->bRespResult = true;
				}
				if (pClient->nType == 10010)
				{
					Json::Value jData;
					if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
					{
						RTC_LOG(LS_ERROR) << "recv data data error";
						return;
					}

					Json::Value jJsep;
					if (!rtc::GetValueFromJsonObject(jData, "jsep", &jJsep))
					{
						RTC_LOG(LS_ERROR) << "recv data jsep error";
						return;
					}

					std::string sdp;
					if (!rtc::GetStringFromJsonObject(jJsep, "sdp", &sdp))
					{
						RTC_LOG(LS_ERROR) << "recv data sdp error";
						return;
					}
					pClient->strSdp = sdp;

					std::string mid;
					if (!rtc::GetStringFromJsonObject(jData, "mid", &mid))
					{
						RTC_LOG(LS_ERROR) << "recv data mid error";
						return;
					}
					pClient->strMid = mid;

					std::string sfu;
					if (!rtc::GetStringFromJsonObject(jData, "sfuid", &sfu))
					{
						RTC_LOG(LS_ERROR) << "recv data sfu error";
						return;
					}
					pClient->sfuId = sfu;

					pClient->bRespResult = true;
				}
				if (pClient->nType == 10015)
				{
					Json::Value jData;
					if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
					{
						RTC_LOG(LS_ERROR) << "recv data data error";
						return;
					}

					Json::Value jJsep;
					if (!rtc::GetValueFromJsonObject(jData, "jsep", &jJsep))
					{
						RTC_LOG(LS_ERROR) << "recv data jsep error";
						return;
					}

					std::string sdp;
					if (!rtc::GetStringFromJsonObject(jJsep, "sdp", &sdp))
					{
						RTC_LOG(LS_ERROR) << "recv data sdp error";
						return;
					}
					pClient->strSdp = sdp;

					std::string sid;
					if (!rtc::GetStringFromJsonObject(jData, "sid", &sid))
					{
						RTC_LOG(LS_ERROR) << "recv data mid error";
						return;
					}
					pClient->strSid = sid;

					pClient->bRespResult = true;
				}
			}
			else
			{
				pClient->nRespOK = 0;
			}
		}
	}
	else
	{
		bool bNotification = false;
		if (!rtc::GetBoolFromJsonObject(jRoot, "notification", &bNotification))
		{
			RTC_LOG(LS_ERROR) << "recv data notification error";
			return;
		}

		if (bNotification)
		{
			std::string method;
			if (!rtc::GetStringFromJsonObject(jRoot, "method", &method))
			{
				RTC_LOG(LS_ERROR) << "recv data method error";
				return;
			}
			if (method == "peer-join")
			{
				Json::Value jData;
				if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
				{
					RTC_LOG(LS_ERROR) << "recv data data error";
					return;
				}

				pClient->pZXEngine->respPeerJoin(jData);
			}
			if (method == "peer-leave")
			{
				Json::Value jData;
				if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
				{
					RTC_LOG(LS_ERROR) << "recv data data error";
					return;
				}

				pClient->pZXEngine->respPeerLeave(jData);
			}
			if (method == "stream-add")
			{
				Json::Value jData;
				if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
				{
					RTC_LOG(LS_ERROR) << "recv data data error";
					return;
				}

				pClient->pZXEngine->respStreamAdd(jData);
			}
			if (method == "stream-remove")
			{
				Json::Value jData;
				if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
				{
					RTC_LOG(LS_ERROR) << "recv data data error";
					return;
				}

				pClient->pZXEngine->respStreamRemove(jData);
			}
			if (method == "peer-kick")
			{
				Json::Value jData;
				if (!rtc::GetValueFromJsonObject(jRoot, "data", &jData))
				{
					RTC_LOG(LS_ERROR) << "recv data data error";
					return;
				}

				pClient->pZXEngine->respPeerKick(jData);
			}
		}
	}
}

void ZXClient::recvThread(ZXClient * client)
{
	if (client != nullptr)
	{
		RTC_LOG(LS_ERROR) << "websocket recv thread start";
		while (client->websocket_ != nullptr && client->websocket_->getReadyState() == WebSocket::OPEN)
		{
			client->websocket_->poll();
			if (client->bClose)
			{
				RTC_LOG(LS_ERROR) << "websocket recv thread stop";
				return;
			}
			client->websocket_->dispatch(OnDataRecv);
		}
		if (!client->bClose)
		{
			client->bClose = true;
			client->pZXEngine->respSocketEvent();
		}
		RTC_LOG(LS_ERROR) << "websocket recv thread stop";
	}
}

bool ZXClient::Start()
{
	Stop();

	RTC_LOG(LS_ERROR) << "websocket url = " << pZXEngine->strUrl;

	mutex_.lock();
	websocket_ = easywsclient::WebSocket::from_url(pZXEngine->strUrl);
	if (websocket_ != nullptr)
	{
		std::thread read_thread_(recvThread, this);
		read_thread_.detach();
		bClose = false;
		mutex_.unlock();
		return true;
	}
	mutex_.unlock();
	return false;
}

void ZXClient::Stop()
{
	bClose = true;
	mutex_.lock();
	if (websocket_ != nullptr)
	{
		websocket_->close();
		//delete websocket_;
		websocket_ = nullptr;
	}
	mutex_.unlock();
}

bool ZXClient::GetConnect()
{
	if (bClose)
	{
		return false;
	}
	if (websocket_ == nullptr)
	{
		return false;
	}

	WebSocket::readyStateValues status = websocket_->getReadyState();
	if (status == WebSocket::readyStateValues::OPEN)
	{
		return true;
	}
	return false;
}

bool ZXClient::SendJoin()
{
	if (bClose || pZXEngine == nullptr)
	{
		return false;
	}

	int nCount = random(1000000, 9000000);
	nIndex = nCount;

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "join";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send join = " << jsonStr;

		nRespOK = -1;
		bRespResult = false;
		nType = 10000;
		websocket_->send(jsonStr);

		int nCount = 30;
		while (nCount > 0)
		{
			if (nRespOK == 0)
			{
				mutex_.unlock();
				return false;
			}
			if (nRespOK == 1)
			{
				if (bRespResult)
				{
					mutex_.unlock();
					return true;
				}
			}
			Sleep(100);
			nCount--;
		}
	}
	mutex_.unlock();
	return false;
}

void ZXClient::SendLeave()
{
	if (bClose || pZXEngine == nullptr)
	{
		return;
	}

	int nCount = random(1000000, 9000000);

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "leave";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonstr;
	std::ostringstream os;
	json::streamwriterbuilder writerbuilder;
	std::unique_ptr<json::streamwriter> jsonwriter(writerbuilder.newstreamwriter());
	jsonwriter->write(jroot, &os);
	jsonstr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send leave = " << jsonStr;
		websocket_->send(jsonStr);
	}
	mutex_.unlock();
}

bool ZXClient::SendAlive()
{
	if (bClose || pZXEngine == nullptr)
	{
		return false;
	}

	int nCount = random(1000000, 9000000);
	nIndex = nCount;

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "keepalive";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send alive = " << jsonStr;

		nRespOK = -1;
		bRespResult = false;
		nType = 10002;
		websocket_->send(jsonStr);

		int nCount = 30;
		while (nCount > 0)
		{
			if (nRespOK == 0)
			{
				mutex_.unlock();
				return false;
			}
			if (nRespOK == 1)
			{
				if (bRespResult)
				{
					mutex_.unlock();
					return true;
				}
			}
			Sleep(100);
			nCount--;
		}
	}
	mutex_.unlock();
	return false;
}

bool ZXClient::SendPublish(std::string sdp, bool bAudio, bool bVideo, int videoType)
{
	if (bClose || pZXEngine == nullptr)
	{
		return false;
	}

	int nCount = random(1000000, 9000000);
	nIndex = nCount;

	Json::Value jsonJsep;
	jsonJsep["sdp"] = sdp;
	jsonJsep["type"] = "offer";

	Json::Value jsonMinfo;
	jsonMinfo["audio"] = bAudio;
	jsonMinfo["video"] = bVideo;
	jsonMinfo["videotype"] = videoType;

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;
	jsonData["jsep"] = jsonJsep;
	jsonData["minfo"] = jsonMinfo;

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "publish";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send publish = " << jsonStr;

		nRespOK = -1;
		bRespResult = false;
		nType = 10010;
		websocket_->send(jsonStr);

		int nCount = 30;
		while (nCount > 0)
		{
			if (nRespOK == 0)
			{
				mutex_.unlock();
				return false;
			}
			if (nRespOK == 1)
			{
				if (bRespResult)
				{
					mutex_.unlock();
					return true;
				}
			}
			Sleep(100);
			nCount--;
		}
	}
	mutex_.unlock();
	return false;
}

void ZXClient::SendUnPublish(std::string mid, std::string sfuid)
{
	if (bClose || pZXEngine == nullptr)
	{
		return;
	}

	int nCount = random(1000000, 9000000);
	
	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;
	jsonData["mid"] = mid;
	if (sfuid != "")
	{
		jsonData["sfuid"] = sfuid;
	}
	
	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "unpublish";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send unpublish = " << jsonStr;
		websocket_->send(jsonStr);
	}
	mutex_.unlock();
}

bool ZXClient::SendSubscribe(std::string sdp, std::string mid, std::string sfuid)
{
	if (bClose || pZXEngine == nullptr)
	{
		return false;
	}

	int nCount = random(1000000, 9000000);
	nIndex = nCount;

	Json::Value jsonJsep;
	jsonJsep["sdp"] = sdp;
	jsonJsep["type"] = "offer";

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;
	jsonData["jsep"] = jsonJsep;
	jsonData["mid"] = mid;
	if (sfuid != "")
	{
		jsonData["sfuid"] = sfuid;
	}

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "subscribe";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send subscribe = " << jsonStr;

		nRespOK = -1;
		bRespResult = false;
		nType = 10015;
		websocket_->send(jsonStr);

		int nCount = 30;
		while (nCount > 0)
		{
			if (nRespOK == 0)
			{
				mutex_.unlock();
				return false;
			}
			if (nRespOK == 1)
			{
				if (bRespResult)
				{
					mutex_.unlock();
					return true;
				}
			}
			Sleep(100);
			nCount--;
		}
	}
	mutex_.unlock();
	return false;
}

void ZXClient::SendUnSubscribe(std::string mid, std::string sid, std::string sfuid)
{
	if (bClose || pZXEngine == nullptr)
	{
		return;
	}

	int nCount = random(1000000, 9000000);

	Json::Value jsonData;
	jsonData["rid"] = pZXEngine->strRid;
	jsonData["mid"] = mid;
	jsonData["sid"] = sid;
	if (sfuid != "")
	{
		jsonData["sfuid"] = sfuid;
	}

	Json::Value jRoot;
	jRoot["request"] = true;
	jRoot["id"] = nCount;
	jRoot["method"] = "unsubscribe";
	jRoot["data"] = jsonData;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	/*std::string jsonStr;
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(jRoot, &os);
	jsonStr = os.str();*/

	mutex_.lock();
	if (websocket_ != nullptr && GetConnect())
	{
		RTC_LOG(LS_ERROR) << "websocket send unsubscribe = " << jsonStr;
		websocket_->send(jsonStr);
	}
	mutex_.unlock();
}