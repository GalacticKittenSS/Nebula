#pragma once

#ifdef NB_WINDOWS

extern Nebula::Application* Nebula::createApplication();

int main(int argc, char** argv) {
	Nebula::Log::Init();
	NB_INFO("Initialised Log!");

	auto app = Nebula::createApplication();
	app->run();
	delete(app);
}

#endif // NB_WINDOWS