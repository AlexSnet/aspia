/*
* PROJECT:         Aspia Remote Desktop
* FILE:            server/client.cpp
* LICENSE:         See top-level directory
* PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
*/

#include "server/client.h"

#include "base/logging.h"

Client::Client(std::unique_ptr<Socket> socket,
               uint32_t client_id,
               OnDisconnectedCallback on_client_disconnected) :
    socket_(std::move(socket)),
    client_id_(client_id),
    on_client_disconnected_(on_client_disconnected),
    feature_mask_(proto::FEATURE_DESKTOP_MANAGE)
{
    DCHECK(socket_);

    std::unique_ptr<proto::ServerToClient> server_message(new proto::ServerToClient());

    proto::AuthRequest *req = server_message->mutable_auth_request();

    req->set_methods(proto::AUTH_NONE | proto::AUTH_BASIC);

    WriteMessage(server_message.get());

    std::unique_ptr<proto::ClientToServer> client_message(new proto::ClientToServer());

    socket_->ReadMessage(&client_message);

    if (!client_message->has_auth_reply())
    {
        throw Exception("Wrong auth reply from client.");
    }

    if (client_message->auth_reply().method() != proto::AUTH_NONE)
    {
        throw Exception("Not supported auth method.");
    }

    server_message->Clear();

    proto::AuthResult *res = server_message->mutable_auth_result();

    res->set_success(true);
    res->set_features(feature_mask_);

    WriteMessage(server_message.get());

    // ���� ������ �� ����� ����������� ������������.
    if (feature_mask_ == proto::FEATURE_NONE)
    {
        // �������� ����������.
        throw Exception("Client session has no features.");
    }
}

Client::~Client()
{
    // Nothing
}

uint32_t Client::GetID() const
{
    return client_id_;
}

void Client::WriteMessage(const proto::ServerToClient *message)
{
    socket_->WriteMessage(message);
}

void Client::Worker()
{
    LOG(INFO) << "Client thread started";

    // ���� ������ ����� ����������� ���������� ������� ������.
    if (feature_mask_ & proto::FEATURE_DESKTOP_MANAGE)
    {
        // �������������� ���������� �����.
        input_handler_.reset(new InputHandler());
    }

    try
    {
        std::unique_ptr<proto::ClientToServer> message(new proto::ClientToServer());

        // ���������� ���� ���� �� ����� ���� ������� ���������� �����.
        while (!IsEndOfThread())
        {
            message->Clear();

            // ������ ��������� �� �������.
            socket_->ReadMessage(&message);

            if (message->has_pointer_event())
            {
                ReadPointerEvent(message->pointer_event());
            }
            else if (message->has_key_event())
            {
                ReadKeyEvent(message->key_event());
            }
            else if (message->has_video_control())
            {
                ReadVideoControl(message->video_control());
            }
            else if (message->has_cursor_shape_control())
            {
                LOG(ERROR) << "CursorShapeControl unimplemented yet";
            }
            else if (message->has_clipboard())
            {
                LOG(ERROR) << "Clipboard unimplemented yet";
            }
            else if (message->has_clipboard_request())
            {
                LOG(ERROR) << "ClipboardRequest unimplemented yet";
            }
            else if (message->has_clipboard_control())
            {
                LOG(ERROR) << "ClipboardControl unimplemented yet";
            }
            else if (message->has_power_control())
            {
                LOG(ERROR) << "PowerControl unimplemented yet";
            }
            else if (message->has_bell())
            {
                LOG(ERROR) << "Bell unimplemented yet";
            }
            else if (message->has_text_chat())
            {
                LOG(ERROR) << "TextChat unimplemented yet";
            }
        }
    }
    catch (const Exception &err)
    {
        LOG(ERROR) << "Exception in client thread: " << err.What();
    }

    // ���� ����� �������� �����-������� ��� ���������������.
    if (screen_sender_)
    {
        // ���� ��� ������� ������������.
        screen_sender_->Stop();

        // ���������� ���������.
        screen_sender_->WaitForEnd();
    }

    // ���� ������� ������������ �������� ������.
    Stop();

    //
    // ���������� ���������� ������� ������� ��������� ������
    // ������������ �������� (����� ������� �� ��������, ���
    // ������ ���������� � ���������� ������� �� ������ ������������
    // �������� "�������").
    //
    std::async(std::launch::async, on_client_disconnected_, GetID());

    LOG(INFO) << "Client thread stopped";
}

void Client::OnStart()
{
    // Nothing
}

void Client::OnStop()
{
    // ��������� ����������.
    socket_->Close();
}

void Client::ReadPointerEvent(const proto::PointerEvent &msg)
{
    //
    // ���� ����� ���������� ������ ����� ���������������.
    // ������ ���� ���������������, ���� ������ ����� �����������
    // ���������� ������� ������, � �������� ������ - �� ���������������.
    //
    if (input_handler_)
    {
        // �������� ������������� ������.
        DesktopRect screen_rect = CapturerGDI::GetDesktopRect();

        // ���� ���������� � ��������� ���������� ������� ��������� � ������� ������.
        if (screen_rect.Contains(msg.x(), msg.y()))
        {
            // ��������� ������� ����������� ������� �/��� ������� ������ ����.
            input_handler_->ExecuteMouse((msg.x() * 65535) / screen_rect.width(),
                                         (msg.y() * 65535) / screen_rect.height(),
                                         msg.mask());
        }
    }
    else
    {
        LOG(ERROR) << "Client has sent a message that he was not allowed to send:"
                   << " PointerEvent. Message ignored.";
    }
}

void Client::ReadKeyEvent(const proto::KeyEvent &msg)
{
    //
    // ���� ����� ���������� ������ ����� ���������������.
    // ������ ���� ���������������, ���� ������ ����� �����������
    // ���������� ������� ������, � �������� ������ - �� ���������������.
    //
    if (input_handler_)
    {
        // ��������� ������� ������� �������.
        input_handler_->ExecuteKeyboard(msg.keycode(), msg.extended(), msg.pressed());
    }
    else
    {
        LOG(ERROR) << "Client has sent a message that he was not allowed to send:"
                   << " KeyEvent. Message ignored.";
    }
}

void Client::ReadVideoControl(const proto::VideoControl &msg)
{
    //
    // ���� ������ ����� ����������� ���������� ������� ������ ���
    // ��������� �������� �����.
    //
    if ((feature_mask_ & proto::FEATURE_DESKTOP_MANAGE) ||
        (feature_mask_ & proto::FEATURE_DESKTOP_VIEW))
    {
        // ���� �������� ������� ��������� �������� �����-�������.
        if (!msg.enable())
        {
            // ���� �������� �����-������� ���� ����������������.
            if (screen_sender_)
            {
                // ���� ������� ���������� ����� �������� �������.
                screen_sender_->Stop();

                // ���������� ���������� ������.
                screen_sender_->WaitForEnd();

                // ���������� ��������� ������.
                screen_sender_.reset();
            }

            // �������, ������ �������� �� ���������.
            return;
        }

        // �� ��������� �������������� ������ �������� ������� � RGB565.
        PixelFormat format = PixelFormat::MakeRGB565();

        // ���� ��� ������� ������ �������� �� �������.
        if (msg.has_pixel_format())
        {
            // �������� ��� � ��������� � ��������� ������.
            const proto::VideoPixelFormat &pf = msg.pixel_format();

            format.set_bits_per_pixel(pf.bits_per_pixel());

            format.set_red_max(pf.red_max());
            format.set_green_max(pf.green_max());
            format.set_blue_max(pf.blue_max());

            format.set_red_shift(pf.red_shift());
            format.set_green_shift(pf.green_shift());
            format.set_blue_shift(pf.blue_shift());
        }

        // ���� �������� �����-������� ��� ����������������.
        if (screen_sender_)
        {
            //
            // ��������������� �� � ������������ � ����������� ��
            // ������� �����������.
            //
            screen_sender_->Configure(msg.encoding(), format);
        }
        else
        {
            ScreenSender::OnMessageAvailabeCallback on_message_available =
                std::bind(&Client::WriteMessage, this, std::placeholders::_1);

            //
            // �������������� �������� �����-������� � �����������
            // �� ������� �����������.
            //
            screen_sender_.reset(new ScreenSender(msg.encoding(),
                                                  format,
                                                  on_message_available));

            // ��������� ����� �������� �����-�������.
            screen_sender_->Start();
        }
    }
    else
    {
        LOG(ERROR) << "Client has sent a message that he was not allowed to send:"
                   << " VideoControl. Message ignored.";
    }
}