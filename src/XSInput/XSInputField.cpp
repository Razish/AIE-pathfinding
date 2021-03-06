#include "XSCommon/XSCommon.h"
#include "XSCommon/XSConsole.h"
#include "XSCommon/XSString.h"
#include "XSInput/XSInputField.h"
#include "XSInput/XSKeys.h"

namespace XS {

	namespace Client {

		std::string InputField::GetHistory( size_t index ) const {
			return history[history.size() - index];
		}

		void InputField::ClampHistory( void ) {
			const size_t historySize = history.size();
			if ( historyIndex > historySize ) {
				historyIndex = historySize;
			}
			// 0 clamp?
		}

		const char *InputField::GetLine( void ) {
			return current.c_str();
		}

		uint32_t InputField::GetCursorPos( void ) const {
			return cursorPos;
		}

		void InputField::Clear( void ) {
			// clear the current state (input line, cursor position, etc)
			current.clear();
			numChars = 0u;
			cursorPos = 0u;
		}

		bool InputField::KeyEvent( SDL_Keycode key, bool down ) {
			if ( char c = GetPrintableCharForKeycode( key ) ) {
				// insert a character at the cursor and move the cursor along
				current.insert( cursorPos, 1, c );
				numChars++;
				cursorPos++;
				return true;
			}
			else if ( key == SDLK_BACKSPACE ) {
				// remove the character before the cursor
				if ( cursorPos != 0 ) {
					current.erase( cursorPos - 1, 1 );
					numChars--;
					cursorPos--;
				}
				return true;
			}
			else if ( key == SDLK_DELETE ) {
				// remove the character after the cursor
				if ( cursorPos != current.length() ) {
					current.erase( cursorPos, 1 );
					numChars--;
				}
			}
			else if ( key == SDLK_RETURN ) {
				// commit the current line to history, pass it to the command buffer and print it to console

				// no action to take if it's empty...
				if ( current.empty() ) {
					return true;
				}

				// pass to command buffer
				if ( execute ) {
					execute( current.c_str() );
				}

				// commit to history
				if ( !current.empty() ) {
					history.push_back( current );
				}

				// make sure history index is reset
				historyIndex = 0u;
				historySeeking = false;

				// print to console
				console.Print( ">%s\n", current.c_str() );

				// clear the state
				Clear();
				return true;
			}
			else if ( key == SDLK_LEFT ) {
				// move cursor left
				if ( cursorPos > 0 ) {
					cursorPos--;
				}
				return true;
			}
			else if ( key == SDLK_RIGHT ) {
				// move cursor right
				if ( cursorPos < numChars ) {
					cursorPos++;
				}
				return true;
			}
			else if ( key == SDLK_UP ) {
				if ( historySeeking ) {
					// if we're already seeking, it means history is not empty
					historyIndex++;
					ClampHistory();
				}
				else {
					// not already seeking, see if we should commit the current line before seeking
					if ( !current.empty() ) {
						history.push_back( current );
					}

					const size_t historySize = history.size();
					if ( historySize ) {
						historySeeking = true; // now we're seeking
						historyIndex++;
						ClampHistory();
					}
				}

				if ( historySeeking ) {
					Clear();
					current = GetHistory( historyIndex );
					// set the cursor pos to the end of the line
					cursorPos = numChars = current.length();
				}

				return true;
			}
			else if ( key == SDLK_DOWN ) {
				if ( historySeeking ) {
					// if we're already seeking, it means history is not empty
					historyIndex--;
					if ( historyIndex == 0u ) {
						historySeeking = false;
					}
					ClampHistory();
				}
				else {
					// not already seeking, see if we should commit the current line
					if ( !current.empty() ) {
						history.push_back( current );
					}
					Clear();
				}

				if ( historySeeking ) {
					current = GetHistory( historyIndex );
					// set the cursor pos to the end of the line
					cursorPos = numChars = current.length();
				}
				else {
					Clear();
				}
				return true;
			}
			else if ( key == SDLK_TAB ) {
				if ( autoComplete ) {
					current = autoComplete( current.c_str() );
				}
				return true;
			}

			return false;
		}

	} // namespace Client

} // namespace XS
