#pragma once

#ifdef NB_WINDOWS

extern Nebula::Application* Nebula::createApplication();

int main(int argc, char** argv) {
	auto app = Nebula::createApplication();
	app->run();
	delete(app);
}

#endif // NB_WINDOWS