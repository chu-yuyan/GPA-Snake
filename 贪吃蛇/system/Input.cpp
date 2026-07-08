#include "Input.h"

#include <graphics.h> // EasyX: ExMessage / peekmessage
#include <windows.h>

InputState Input::poll() const
{
	InputState st{};

	ExMessage msg{};
	while (peekmessage(&msg, EX_KEY, true))
	{
		if (msg.message != WM_KEYDOWN)
			continue;

		switch (msg.vkcode)
		{
		case VK_ESCAPE:
			st.quit = true;
			break;

		case 'W': st.dirChanged = true; st.dir = Direction::Up; break;
		case 'S': st.dirChanged = true; st.dir = Direction::Down; break;
		case 'A': st.dirChanged = true; st.dir = Direction::Left; break;
		case 'D': st.dirChanged = true; st.dir = Direction::Right; break;

		case VK_UP:    st.dirChanged = true; st.dir = Direction::Up; break;
		case VK_DOWN:  st.dirChanged = true; st.dir = Direction::Down; break;
		case VK_LEFT:  st.dirChanged = true; st.dir = Direction::Left; break;
		case VK_RIGHT: st.dirChanged = true; st.dir = Direction::Right; break;

		default:
			break;
		}
	}

	st.spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

	return st;
}