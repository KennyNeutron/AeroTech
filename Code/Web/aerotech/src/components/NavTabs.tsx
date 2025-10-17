"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";

const tabs = [
  { name: "Home", href: "/home", icon: "🏠" },
  { name: "Sensors", href: "/sensors", icon: "🧭" },
  { name: "Actuators", href: "/actuators", icon: "⚡" },
  { name: "Settings", href: "/settings", icon: "⚙️" },
];

export default function NavTabs() {
  const pathname = usePathname();

  return (
    <nav className="w-full">
      <div className="mx-auto max-w-3xl">
        <div className="rounded-full bg-brand-100 p-1 flex gap-1">
          {tabs.map((t) => {
            const active = pathname === t.href;
            return (
              <Link
                key={t.href}
                href={t.href}
                className={`flex-1 text-center rounded-full px-4 py-2 text-sm font-medium transition
                  ${
                    active
                      ? "bg-white text-brand-700 shadow"
                      : "text-brand-800/70 hover:text-brand-800 hover:bg-white/60"
                  }`}
              >
                <span className="mr-2">{t.icon}</span>
                {t.name}
              </Link>
            );
          })}
        </div>
      </div>
    </nav>
  );
}
