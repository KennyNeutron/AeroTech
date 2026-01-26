export const dynamic = "force-dynamic";
export const revalidate = 0;

import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";

export default async function HomePage() {
  return (
    <main className="min-h-screen bg-brand-50 pb-12">
      <AppHeader />

      <div className="px-4 mb-8">
        <NavTabs />
      </div>

      <div className="max-w-4xl mx-auto px-6">
        {/* Hero Section */}
        <section className="text-center mb-16 animate-in fade-in slide-in-from-bottom-4 duration-1000">
          <h1 className="text-4xl md:text-5xl font-extrabold text-brand-900 leading-tight mb-6 mt-8">
            AEROTECH: Cultivating the Future of{" "}
            <span className="text-brand-600">Sustainable Agriculture.</span>
          </h1>
          <div className="w-24 h-1.5 bg-brand-500 mx-auto rounded-full"></div>
        </section>

        {/* Project Overview */}
        <section className="mb-12 bg-white rounded-3xl p-8 shadow-card border border-brand-100">
          <h2 className="text-sm font-bold tracking-widest text-brand-600 uppercase mb-4">
            Project Overview
          </h2>
          <p className="text-lg text-brand-800 leading-relaxed">
            AeroTech is a cutting-edge, sensor-based aeroponics system designed
            to revolutionize how we grow vegetables. By utilizing
            microcontroller-triggered automation, our system creates the perfect
            environment for plants to thrive without soil. From real-time
            climate control to precision nutrient delivery, AeroTech integrates
            advanced technology with environmental responsibility. Powered by
            solar energy and monitored via the cloud, we are making
            high-efficiency, sustainable farming accessible and reliable.
          </p>
        </section>

        <div className="grid md:grid-cols-2 gap-8">
          {/* Mission */}
          <section className="bg-brand-900 text-white rounded-3xl p-8 shadow-card flex flex-col justify-between">
            <div>
              <h2 className="text-sm font-bold tracking-widest text-brand-300 uppercase mb-4">
                Mission
              </h2>
              <blockquote className="text-xl font-semibold mb-6 italic border-l-4 border-brand-400 pl-4">
                "To empower modern agriculture through smart automation and
                sustainable energy."
              </blockquote>
              <p className="text-brand-100 leading-relaxed">
                Our mission is to design and implement a reliable,
                self-sustaining aeroponics system that maximizes growth
                efficiency while minimizing resource waste. We strive to provide
                growers with real-time data and remote accessibility, ensuring
                that food production is consistent, data-driven, and
                environmentally friendly.
              </p>
            </div>
          </section>

          {/* Vision */}
          <section className="bg-white border-2 border-brand-200 rounded-3xl p-8 shadow-card flex flex-col justify-between">
            <div>
              <h2 className="text-sm font-bold tracking-widest text-brand-600 uppercase mb-4">
                Vision
              </h2>
              <blockquote className="text-xl font-semibold text-brand-900 mb-6 italic border-l-4 border-brand-500 pl-4">
                "To lead the transition toward a global, tech-driven sustainable
                food system."
              </blockquote>
              <p className="text-brand-800 leading-relaxed">
                We envision a world where anyone, anywhere, can grow
                high-quality produce with minimal environmental impact. By
                combining cloud analytics and renewable power, AeroTech aims to
                set the standard for the next generation of smart urban farming
                and food security.
              </p>
            </div>
          </section>
        </div>

        {/* Proponents Section */}
        <section className="mt-24 border-t border-brand-200 pt-16">
          <h2 className="text-center text-2xl font-bold text-brand-900 mb-12 italic tracking-tight">
            The Proponents
          </h2>
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-12 max-w-3xl mx-auto">
            {[
              {
                name: "JHANN RIEL E. SALINAS",
                avatar: "/Proponents/SALINAS.jpg",
              },
              { name: "GHAZAAL J. AZAD", avatar: "/Proponents/AZAD.jpeg" },
              {
                name: "RALPH DAVE F. OREDINA",
                avatar: "/Proponents/OREDINA.png",
              },
            ].map((proponent) => (
              <div
                key={proponent.name}
                className="flex flex-col items-center text-center group"
              >
                <div className="relative w-32 h-32 mb-6 transition-all duration-500 group-hover:scale-105">
                  <div className="absolute inset-0 bg-brand-500 rounded-full blur-2xl opacity-0 group-hover:opacity-20 transition-opacity"></div>
                  <img
                    src={proponent.avatar}
                    alt={proponent.name}
                    className="relative w-full h-full object-cover rounded-full border-4 border-white shadow-xl ring-1 ring-brand-100"
                  />
                </div>
                <h3 className="text-[11px] font-black text-brand-900 tracking-[0.2em] uppercase px-2 leading-tight">
                  {proponent.name}
                </h3>
                <p className="text-[10px] text-brand-600 font-medium tracking-widest uppercase mt-1">
                  Project Engineer
                </p>
                <div className="w-6 h-0.5 bg-brand-500 mt-2 rounded-full group-hover:w-12 transition-all duration-300"></div>
              </div>
            ))}
          </div>
        </section>
      </div>
    </main>
  );
}
